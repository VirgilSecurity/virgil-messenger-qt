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

#include "VSQDownload.h"
#include "VSQSettings.h"
#include "VSQUpload.h"

Q_LOGGING_CATEGORY(lcTransferManager, "transferman");

VSQTransferManager::VSQTransferManager(QXmppClient *client, QNetworkAccessManager *networkAccessManager, VSQSettings *settings, QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(networkAccessManager)
    , m_settings(settings)
    , m_xmppManager(new QXmppUploadRequestManager())
{
    qRegisterMetaType<QXmppHttpUploadSlotIq>();
    qRegisterMetaType<QXmppHttpUploadRequestIq>();

    connect(this, &VSQTransferManager::requestUploadUrl, this, &VSQTransferManager::onRequestUploadUrl);
    connect(this, &VSQTransferManager::startUpload, this, &VSQTransferManager::onStartUpload);
    connect(this, &VSQTransferManager::startDownload, this, &VSQTransferManager::onStartDownload);

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

void VSQTransferManager::startTransfer(VSQTransfer *transfer)
{
    connect(transfer, &VSQTransfer::progressChanged, this,
            std::bind(&VSQTransferManager::progressChanged, this, transfer->messageId(), args::_1, args::_2));
    connect(transfer, &VSQTransfer::finished, this,
            std::bind(&VSQTransferManager::statusChanged, this, transfer->messageId(), Attachment::Status::Loaded));
    connect(transfer, &VSQTransfer::failed, this,
            std::bind(&VSQTransferManager::statusChanged, this, transfer->messageId(), Attachment::Status::Failed));
    if (auto download = qobject_cast<VSQDownload *>(transfer)) {
        connect(download, &VSQDownload::finished, this,
                std::bind(&VSQTransferManager::fileDownloaded, this, download->messageId(), download->attachment().encLocalUrl));
    }
    transfer->start();
}

void VSQTransferManager::onRequestUploadUrl(const QString &messageId, const QString &fileName)
{
    if (m_xmppManager->serviceFound()) {
        auto slotId = m_xmppManager->requestUploadSlot(QFileInfo(fileName));
        m_transfers.push_back(new VSQUpload(m_networkAccessManager, messageId, slotId, this));
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
    // Upload was added in url request
    if (auto transfer = findTransferByMessageId(messageId)) {
        auto upload = qobject_cast<VSQUpload *>(transfer);
        if (!upload) {
            qCWarning(lcTransferManager) << "Message" << messageId << "has no upload";
            return;
        }
        upload->setAttachment(attachment);
        startTransfer(upload);
    }
    else {
        qCWarning(lcTransferManager) << "Message" << messageId << "has no upload";
    }
}

void VSQTransferManager::onStartDownload(const QString &messageId, const Attachment &attachment)
{
    auto download = new VSQDownload(m_networkAccessManager, messageId, m_settings->attachmentCacheDir(), this);
    m_transfers.push_back(download);
    download->setAttachment(attachment);
    startTransfer(download);
}
