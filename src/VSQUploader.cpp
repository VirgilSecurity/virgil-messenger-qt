//  Copyright (C) 2015-2020 Virgil Security, Inc.
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      (1) Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      (2) Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//      (3) Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
//  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
//  Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

#include "VSQUploader.h"

#include <QXmppClient.h>
#include <QXmppUploadRequestManager.h>

#include "VSQUpload.h"

Q_LOGGING_CATEGORY(lcUploader, "uploader");

VSQUploader::VSQUploader(QXmppClient *client, QNetworkAccessManager *networkAccessManager, QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(networkAccessManager)
    , m_xmppManager(new QXmppUploadRequestManager())
{
    qRegisterMetaType<QXmppHttpUploadSlotIq>();
    qRegisterMetaType<QXmppHttpUploadRequestIq>();

    connect(this, &VSQUploader::requestUrl, this, &VSQUploader::onRequestUrl);
    connect(this, &VSQUploader::startUpload, this, &VSQUploader::onStartUpload);

    client->addExtension(m_xmppManager);
    connect(m_xmppManager, &QXmppUploadRequestManager::slotReceived, this, &VSQUploader::onSlotReceived);
    connect(m_xmppManager, &QXmppUploadRequestManager::requestFailed, this, &VSQUploader::onRequestFailed);

    qCDebug(lcUploader) << "Service found:" << m_xmppManager->serviceFound();
    connect(m_xmppManager, &QXmppUploadRequestManager::serviceFoundChanged, this, [&]() {
        qCDebug(lcUploader) << "Upload service found:" << m_xmppManager->serviceFound();
    });
}

VSQUploader::~VSQUploader()
{
    while (!m_uploads.empty()) {
        abortUpload(m_uploads.first());
    }
    delete m_xmppManager;
}

VSQUpload *VSQUploader::findUploadBySlotId(const QString &slotId)
{
    for (auto upload : m_uploads) {
        if (upload->slotId() == slotId) {
            return upload;
        }
    }
    qCWarning(lcUploader) << "Upload wasn't found. Slot id:" << slotId;
    return nullptr;
}

VSQUpload *VSQUploader::findUploadByMessageId(const QString &messageId)
{
    for (auto upload : m_uploads) {
        if (upload->messageId() == messageId) {
            return upload;
        }
    }
    qCWarning(lcUploader) << "Upload wasn't found. Message id:" << messageId;
    return nullptr;
}

void VSQUploader::removeUpload(VSQUpload *upload)
{
    m_uploads.removeOne(upload);
    upload->deleteLater();
}

void VSQUploader::abortUpload(VSQUpload *upload)
{
    upload->abort();
    removeUpload(upload);
}

void VSQUploader::onRequestUrl(const QString &messageId, const QString &fileName)
{
    if (m_xmppManager->serviceFound()) {
        auto slotId = m_xmppManager->requestUploadSlot(QFileInfo(fileName));
        m_uploads.push_back(new VSQUpload(m_networkAccessManager, messageId, slotId, fileName, this));
    }
    else {
        qCDebug(lcUploader) << "Upload service was not found";
        emit statusChanged(messageId, Attachment::Status::Failed);
    }
}

void VSQUploader::onSlotReceived(const QXmppHttpUploadSlotIq &slot)
{
    qCDebug(lcUploader) << "VSQUploader::onSlotReceived";
    if (auto upload = findUploadBySlotId(slot.id())) {
        emit urlReceived(upload->messageId(), slot.putUrl());
        emit statusChanged(upload->messageId(), Attachment::Status::Loading);
    }
}

void VSQUploader::onRequestFailed(const QXmppHttpUploadRequestIq &request)
{
    qCDebug(lcUploader) << "VSQUploader::onRequestFailed" << request.error().text();
    if (auto upload = findUploadBySlotId(request.id())) {
        emit urlErrorOccured(upload->messageId());
        removeUpload(upload);
    }
}

void VSQUploader::onStartUpload(const QString &messageId, const Attachment &attachment)
{
    if (auto upload = findUploadByMessageId(messageId)) {
        upload->setAttachment(attachment);
        connect(upload, &VSQUpload::progressChanged, this, std::bind(&VSQUploader::progressChanged, this, messageId, args::_1, args::_2));
        connect(upload, &VSQUpload::finished, this, std::bind(&VSQUploader::statusChanged, this, messageId, Attachment::Status::Loaded));
        connect(upload, &VSQUpload::failed, this, std::bind(&VSQUploader::statusChanged, this, messageId, Attachment::Status::Failed));
        upload->start();
    }
}
