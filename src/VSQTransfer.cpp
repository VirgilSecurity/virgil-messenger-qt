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

VSQTransfer::VSQTransfer(QNetworkAccessManager *networkAccessManager, const QString &messageId, QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(networkAccessManager)
    , m_running(false)
    , m_messageId(messageId)
{}

VSQTransfer::~VSQTransfer()
{
    // FIXME(fpohtmeh): remove encrypted local file on success
}

QString VSQTransfer::messageId() const
{
    return m_messageId;
}

void VSQTransfer::setAttachment(const Attachment &attachment)
{
    m_attachment = attachment;
}

const Attachment VSQTransfer::attachment() const
{
    return m_attachment;
}

void VSQTransfer::start()
{}

void VSQTransfer::abort()
{
    if (!m_running) {
        return;
    }
    m_running = false;
    const QString message = QString("Aborted transfer %1 / %2 / %3").arg(
                messageId(), m_attachment.localUrl.toLocalFile(), m_attachment.remoteUrl.toString());
    qCDebug(lcTransferManager) << message;
    emit failed(message);
}

void VSQTransfer::connectReply(QNetworkReply *reply)
{
    qCDebug(lcDev) << "Connected to reply:" << reply;
    connect(reply, &QNetworkReply::uploadProgress, this, &VSQTransfer::progressChanged);
    connect(reply, &QNetworkReply::finished, this, &VSQTransfer::finished);
    connect(reply, &QNetworkReply::errorOccurred, this, std::bind(&VSQTransfer::onNetworkReplyError, this, args::_1, reply));
    connect(this, &VSQTransfer::finished, reply, std::bind(&VSQTransfer::cleanupReply, this, reply));
    connect(this, &VSQTransfer::failed, reply, std::bind(&VSQTransfer::cleanupReply, this, reply));
}

void VSQTransfer::onNetworkReplyError(QNetworkReply::NetworkError error, QNetworkReply *reply)
{
    emit failed(QString("Network error (code %1): %2").arg(error).arg(reply->errorString()));
}

void VSQTransfer::cleanupReply(QNetworkReply *reply)
{
    m_running = false;
    reply->deleteLater();
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "Cleanuped reply:" << reply;
#endif
}

QFile *VSQTransfer::getAttachmentFile()
{
    auto file = new QFile(m_attachment.filePath(), this);
    connect(this, &VSQTransfer::finished, file, &QFile::close);
    connect(this, &VSQTransfer::failed, file, &QFile::close);
    return file;
}
