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
    , m_status(Attachment::Status::Created)
{
    connect(this, &VSQTransfer::progressChanged, [=](const DataSize bytesReceived, const DataSize bytesTotal) {
        if (bytesReceived == bytesTotal) {
            setStatus(Attachment::Status::Loaded);
        }
    });
    connect(this, &VSQTransfer::statusChanged, [=](const Enums::AttachmentStatus status){
        if (status == Attachment::Status::Loaded || status == Attachment::Status::Failed) {
            emit ended(status == Attachment::Status::Failed);
        }
    });
}

VSQTransfer::~VSQTransfer()
{
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
    return m_status == Attachment::Status::Loading;
}

void VSQTransfer::start()
{
    setStatus(Attachment::Status::Loading);
}

void VSQTransfer::abort()
{
    if (!isRunning()) {
        return;
    }
    qCDebug(lcTransferManager) << QString("Aborted transfer %1").arg(id());
    setStatus(Attachment::Status::Failed);
}

void VSQTransfer::connectReply(QNetworkReply *reply)
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "Connected to reply:" << reply;
#endif
    connect(reply, &QNetworkReply::finished, [=]() {
        setStatus(Attachment::Status::Loaded);
    });
    connect(reply, &QNetworkReply::errorOccurred, [=](QNetworkReply::NetworkError error) {
        qCDebug(lcTransferManager) << QString("Network error (code %1): %2").arg(error).arg(reply->errorString());
        setStatus(Attachment::Status::Failed);
    });
    connect(reply, &QNetworkReply::sslErrors, [=]() {
        qCDebug(lcTransferManager) << "SSL errors";
        setStatus(Attachment::Status::Failed);
    });
}

void VSQTransfer::setStatus(const Attachment::Status status)
{
    if (status == m_status) {
        return;
    }
    m_status = status;
    emit statusChanged(status);
}

QNetworkAccessManager *VSQTransfer::networkAccessManager()
{
    return m_networkAccessManager;
}

QFile *VSQTransfer::fileHandle(const QString &filePath)
{
    auto file = new QFile(filePath);
    connect(this, &VSQTransfer::ended, file, &QFile::close);
    connect(this, &VSQTransfer::ended, file, &QFile::deleteLater);
    return file;
}
