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

#include "VSQTransferManager.h"

#include <QXmppClient.h>
#include <QXmppUploadRequestManager.h>

#include "VSQUpload.h"

Q_LOGGING_CATEGORY(lcTransferManager, "transferman");

VSQTransferManager::VSQTransferManager(QXmppClient *client, QNetworkAccessManager *networkAccessManager, QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(networkAccessManager)
    , m_xmppManager(new QXmppUploadRequestManager())
{
    qRegisterMetaType<QXmppHttpUploadSlotIq>();
    qRegisterMetaType<QXmppHttpUploadRequestIq>();

    connect(this, &VSQTransferManager::requestUploadUrl, this, &VSQTransferManager::onRequestUploadUrl);
    connect(this, &VSQTransferManager::startUpload, this, &VSQTransferManager::onStartUpload);

    m_xmppManager->setParent(this);
    client->addExtension(m_xmppManager);
    connect(m_xmppManager, &QXmppUploadRequestManager::slotReceived, this, &VSQTransferManager::onSlotReceived);
    connect(m_xmppManager, &QXmppUploadRequestManager::requestFailed, this, &VSQTransferManager::onRequestFailed);

    qCDebug(lcTransferManager) << "Service found:" << m_xmppManager->serviceFound();
    connect(m_xmppManager, &QXmppUploadRequestManager::serviceFoundChanged, this, [&]() {
        qCDebug(lcTransferManager) << "Upload service found:" << m_xmppManager->serviceFound();
    });
}

VSQTransferManager::~VSQTransferManager()
{
    while (!m_transfers.empty()) {
        abortTransfer(m_transfers.first());
    }
}

VSQTransfer *VSQTransferManager::findUploadBySlotId(const QString &slotId)
{
    for (auto transfer : m_transfers) {
        auto upload = qobject_cast<VSQUpload *>(transfer);
        if (upload && upload->slotId() == slotId) {
            return upload;
        }
    }
    qCWarning(lcTransferManager) << "Upload wasn't found. Slot id:" << slotId;
    return nullptr;
}

VSQTransfer *VSQTransferManager::findTransferByMessageId(const QString &messageId)
{
    for (auto transfer : m_transfers) {
        if (transfer->messageId() == messageId) {
            return transfer;
        }
    }
    qCWarning(lcTransferManager) << "Transfer wasn't found. Message id:" << messageId;
    return nullptr;
}

void VSQTransferManager::removeTransfer(VSQTransfer *transfer)
{
    m_transfers.removeOne(transfer);
    transfer->deleteLater();
}

void VSQTransferManager::abortTransfer(VSQTransfer *transfer)
{
    transfer->abort();
    removeTransfer(transfer);
}

void VSQTransferManager::onRequestUploadUrl(const QString &messageId, const QString &fileName)
{
    if (m_xmppManager->serviceFound()) {
        auto slotId = m_xmppManager->requestUploadSlot(QFileInfo(fileName));
        m_transfers.push_back(new VSQUpload(m_networkAccessManager, messageId, slotId, fileName, this));
    }
    else {
        qCDebug(lcTransferManager) << "Upload service was not found";
        emit statusChanged(messageId, Attachment::Status::Failed);
    }
}

void VSQTransferManager::onSlotReceived(const QXmppHttpUploadSlotIq &slot)
{
    qCDebug(lcTransferManager) << "VSQUploader::onSlotReceived";
    if (auto upload = findUploadBySlotId(slot.id())) {
        emit uploadUrlReceived(upload->messageId(), slot.putUrl());
        emit statusChanged(upload->messageId(), Attachment::Status::Loading);
    }
}

void VSQTransferManager::onRequestFailed(const QXmppHttpUploadRequestIq &request)
{
    qCDebug(lcTransferManager) << "VSQUploader::onRequestFailed" << request.error().text();
    if (auto upload = findUploadBySlotId(request.id())) {
        emit uploadUrlErrorOccured(upload->messageId());
        removeTransfer(upload);
    }
}

void VSQTransferManager::onStartUpload(const QString &messageId, const Attachment &attachment)
{
    if (auto transfer = findTransferByMessageId(messageId)) {
        auto upload = qobject_cast<VSQUpload *>(transfer);
        if (!upload) {
            qCWarning(lcTransferManager) << "Message" << messageId << "has no upload";
            return;
        }
        upload->setAttachment(attachment);
        connect(upload, &VSQUpload::progressChanged, this, std::bind(&VSQTransferManager::progressChanged, this, messageId, args::_1, args::_2));
        connect(upload, &VSQUpload::finished, this, std::bind(&VSQTransferManager::statusChanged, this, messageId, Attachment::Status::Loaded));
        connect(upload, &VSQUpload::failed, this, std::bind(&VSQTransferManager::statusChanged, this, messageId, Attachment::Status::Failed));
        upload->start();
    }
}
