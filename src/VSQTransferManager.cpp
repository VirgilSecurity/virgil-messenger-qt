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
#include "Settings.h"
#include "VSQUpload.h"

#include <QTimer>
#include <QEventLoop>

Q_LOGGING_CATEGORY(lcTransferManager, "transferman");

VSQTransferManager::VSQTransferManager(QXmppClient *client, QNetworkAccessManager *networkAccessManager, VSQSettings *settings, QObject *parent)
    : QObject(parent)
    , m_client(client)
    , m_networkAccessManager(networkAccessManager)
    , m_settings(settings)
    , m_xmppManager(new QXmppUploadRequestManager())
{
    qRegisterMetaType<QXmppHttpUploadSlotIq>();
    qRegisterMetaType<QXmppHttpUploadRequestIq>();

    connect(this, &VSQTransferManager::startTransfer, this, &VSQTransferManager::onStartTransfer);

    m_xmppManager->setParent(this);
    client->addExtension(m_xmppManager);
    connect(m_xmppManager, &QXmppUploadRequestManager::slotReceived, this, &VSQTransferManager::onSlotReceived);
    connect(m_xmppManager, &QXmppUploadRequestManager::requestFailed, this, &VSQTransferManager::onRequestFailed);

    connect(client, &QXmppClient::connected, this, &VSQTransferManager::connectionChanged);
    connect(client, &QXmppClient::disconnected, this, &VSQTransferManager::connectionChanged);
    connect(client, &QXmppClient::error, this, &VSQTransferManager::connectionChanged);

    qCDebug(lcTransferManager) << "Service found:" << m_xmppManager->serviceFound();
    connect(m_xmppManager, &QXmppUploadRequestManager::serviceFoundChanged, this, [=]() {
        bool ready = m_xmppManager->serviceFound();
        qCDebug(lcTransferManager) << "Upload service found:" << ready;
        if (ready) {
            emit fireReadyToUpload();
        }
    });
}

VSQTransferManager::~VSQTransferManager()
{
    QMutexLocker locker(&m_transfersMutex);
    while (!m_transfers.empty()) {
        abortTransfer(m_transfers.first(), false);
    }
}

bool VSQTransferManager::isReady() const
{
    if (m_xmppManager->serviceFound()) {
        return true;
    }

    qCDebug(lcTransferManager) << "Upload service was not found, start waiting for it.";
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(this, &VSQTransferManager::fireReadyToUpload, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();
    qCDebug(lcTransferManager) << "Upload service found:" << m_xmppManager->serviceFound();
    return m_xmppManager->serviceFound();
}

bool VSQTransferManager::hasTransfer(const QString &id) const
{
    return findTransfer(id, false) != nullptr;
}

VSQSettings *VSQTransferManager::settings()
{
    return m_settings;
}

VSQUpload *VSQTransferManager::startUpload(const QString &id, const QString &filePath)
{
    auto upload = new VSQUpload(m_networkAccessManager, id, filePath, nullptr);
    {
        QMutexLocker locker(&m_transfersMutex);
        m_transfers.push_back(upload);
    }
    if (!requestUploadUrl(upload)) {
        removeTransfer(upload, true);
        return nullptr;
    }
    startTransfer(upload, QPrivateSignal());
    return upload;
}

VSQDownload *VSQTransferManager::startDownload(const QString &id, const QUrl &remoteUrl, const QString &filePath)
{
    auto download = new VSQDownload(m_networkAccessManager, id, remoteUrl, filePath, nullptr);
    {
        QMutexLocker locker(&m_transfersMutex);
        m_transfers.push_back(download);
    }
    startTransfer(download, QPrivateSignal());
    return download;
}

bool VSQTransferManager::requestUploadUrl(VSQUpload *upload)
{
    const auto filePath = upload->filePath();
    if (!QFile::exists(filePath)) {
        qCCritical(lcTransferManager) << "Uploaded file doesn't exist:" << filePath;
        upload->setStatus(AttachmentV0::Status::Failed);
    }
    else if (isReady()) {
        auto slotId = m_xmppManager->requestUploadSlot(QFileInfo(filePath));
        upload->setSlotId(slotId);
        return true;
    } else {
        qCDebug(lcTransferManager) << "Upload service was not found";
        upload->setStatus(AttachmentV0::Status::Failed);
    }

    return false;
}

VSQUpload *VSQTransferManager::findUploadBySlotId(const QString &slotId) const
{
    {
        QMutexLocker locker(&m_transfersMutex);
        for (auto transfer : m_transfers) {
            auto upload = qobject_cast<VSQUpload *>(transfer);
            if (upload && upload->slotId() == slotId) {
                return upload;
            }
        }
    }
    qCWarning(lcTransferManager) << "Upload wasn't found. Slot id:" << slotId;
    return nullptr;
}

VSQTransfer *VSQTransferManager::findTransfer(const QString &id, bool showWarning) const
{
    {
        QMutexLocker locker(&m_transfersMutex);
        for (auto transfer : m_transfers) {
            if (transfer->id() == id) {
                return transfer;
            }
        }
    }
    if (showWarning) {
        qCWarning(lcTransferManager) << "Transfer wasn't found:" << id;
    }
    return nullptr;
}

void VSQTransferManager::removeTransfer(VSQTransfer *transfer, bool lock)
{
    qCDebug(lcTransferManager) << "Removing of transfer" << transfer->id();
    if (!lock) {
        m_transfers.removeOne(transfer);
    }
    else {
        QMutexLocker locker(&m_transfersMutex);
        m_transfers.removeOne(transfer);
    }
    QTimer::singleShot(1000, transfer, &VSQTransfer::deleteLater); // HACK(fpohtmeh): remove transfer later
}

void VSQTransferManager::abortTransfer(VSQTransfer *transfer, bool lock)
{
    qCDebug(lcTransferManager) << "Aborting of transfer" << transfer->id();
    transfer->abort();
    if (!lock) {
        removeTransfer(transfer, false);
    }
    else {
        QMutexLocker locker(&m_transfersMutex);
        removeTransfer(transfer, false);
    }
}

void VSQTransferManager::onStartTransfer(VSQTransfer *transfer)
{
    connect(transfer, &VSQTransfer::progressChanged, this,
            std::bind(&VSQTransferManager::progressChanged, this, transfer->id(), args::_1, args::_2));
    connect(transfer, &VSQTransfer::statusChanged, this,
            std::bind(&VSQTransferManager::statusChanged, this, transfer->id(), args::_1));
    connect(transfer, &VSQTransfer::ended, this,
            std::bind(&VSQTransferManager::removeTransfer, this, transfer, true));
    connect(this, &VSQTransferManager::connectionChanged, transfer, &VSQTransfer::connectionChanged);
    transfer->start();
}

void VSQTransferManager::onSlotReceived(const QXmppHttpUploadSlotIq &slot)
{
    qCDebug(lcTransferManager) << "VSQUploader::onSlotReceived";
    if (auto upload = findUploadBySlotId(slot.id())) {
        qCDebug(lcTransferManager) << "Remote url was received for" << upload->id();
        upload->remoteUrlReceived(slot.putUrl());
    }
}

void VSQTransferManager::onRequestFailed(const QXmppHttpUploadRequestIq &request)
{
    qCDebug(lcTransferManager) << "VSQUploader::onRequestFailed" << request.error().text();
    if (auto upload = findUploadBySlotId(request.id())) {
        qCWarning(lcTransferManager) << "Remote url error occured for" << upload->id();
        upload->remoteUrlErrorOccured();
        removeTransfer(upload, true);
    }
}
