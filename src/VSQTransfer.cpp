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

#include "VSQTransfer.h"

VSQTransfer::VSQTransfer(QNetworkAccessManager *networkAccessManager, const QString &id, QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(networkAccessManager)
    , m_id(id)
    , m_status(AttachmentV0::Status::Created)
{
    connect(this, &VSQTransfer::progressChanged, [&](const DataSize bytesReceived, const DataSize bytesTotal) {
        if (bytesTotal == 0) {
            return; // HACK(fpohtmeh): reply sends zeros finally, ignore it
        }
        m_bytesReceived = bytesReceived;
        m_bytesTotal = bytesTotal;
        if (m_bytesTotal > 0 && m_bytesReceived >= m_bytesTotal) {
            qCDebug(lcTransferManager) << "All bytes were processed, mark transfer as completed";
            closeFileHandle();
            setStatus(AttachmentV0::Status::Loaded);
        }
    });
    connect(this, &VSQTransfer::statusChanged, [=](const Enums::AttachmentStatus status){
        if (status == AttachmentV0::Status::Loaded || status == AttachmentV0::Status::Failed) {
            closeFileHandle();
            emit ended(status == AttachmentV0::Status::Failed);
        }
    });
}

VSQTransfer::~VSQTransfer()
{
    closeFileHandle();
#ifdef VS_DEVMODE
    qCDebug(lcTransferManager) << "Deletion of transfer:" << id();
#endif
}

QString VSQTransfer::id() const
{
    return m_id;
}

bool VSQTransfer::isRunning() const
{
    return m_status == AttachmentV0::Status::Loading;
}

bool VSQTransfer::isFailed() const
{
    return m_status == AttachmentV0::Status::Failed;
}

void VSQTransfer::start()
{
    setStatus(AttachmentV0::Status::Loading);
}

void VSQTransfer::abort()
{
    if (!isRunning()) {
        qCDebug(lcTransferManager) << "Can't abort not running transfer";
        return;
    }
    qCDebug(lcTransferManager) << QString("Aborted transfer %1").arg(id());
    setStatus(AttachmentV0::Status::Failed);
}

QList<QMetaObject::Connection> VSQTransfer::connectReply(QNetworkReply *reply, QMutex *guard)
{
    QList<QMetaObject::Connection> res;
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "Connected to reply:" << reply;
#endif
    res << connect(reply, &QNetworkReply::finished, [=]() {
        QMutexLocker l(guard);
        if (m_bytesTotal > 0 && m_bytesReceived >= m_bytesTotal) {
            setStatus(AttachmentV0::Status::Loaded);
        }
        else {
            qCDebug(lcTransferManager) << "Failed. File was processed partially";
            setStatus(AttachmentV0::Status::Failed);
        }
    });
    res << connect(reply, &QNetworkReply::errorOccurred, [=](QNetworkReply::NetworkError error) {
        QMutexLocker l(guard);
        qCDebug(lcTransferManager) << QString("Network error (code %1): %2").arg(error).arg(reply->errorString());
        setStatus(AttachmentV0::Status::Failed);
    });
    res << connect(reply, &QNetworkReply::sslErrors, [=]() {
        QMutexLocker l(guard);
        qCDebug(lcTransferManager) << "SSL errors";
        setStatus(AttachmentV0::Status::Failed);
    });

    return res;
}

void VSQTransfer::setStatus(const AttachmentV0::Status status)
{
    if (status == m_status) {
        return;
    }
    qCDebug(lcTransferManager) << "Transfer" << id() << "status:" << status;
    m_status = status;
    emit statusChanged(status);
}

QNetworkAccessManager *VSQTransfer::networkAccessManager()
{
    return m_networkAccessManager;
}

QFile *VSQTransfer::createFileHandle(const QString &filePath)
{
    if (m_fileHandle) {
        qCWarning(lcTransferManager) << "File handle already exists for:" << filePath;
        return nullptr;
    }
    m_fileHandle = new QFile(filePath);
    return m_fileHandle;
}

void VSQTransfer::closeFileHandle()
{
    if (!m_fileHandle) {
        return;
    }
    const auto fileName = m_fileHandle->fileName();
    m_fileHandle->close();
    m_fileHandle->deleteLater();
    m_fileHandle = nullptr;
    qCDebug(lcTransferManager) << "Closed file handle:" << fileName << "size:" << QFileInfo(fileName).size();
}
