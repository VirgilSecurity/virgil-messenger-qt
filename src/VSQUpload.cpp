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

#include "VSQUpload.h"

#include <QMimeDatabase>
#include <QNetworkReply>

VSQUpload::VSQUpload(QNetworkAccessManager *networkAccessManager, const QString &messageId, const QString &slotId, const QString &fileName, QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(networkAccessManager)
    , m_messageId(messageId)
    , m_slotId(slotId)
    , m_fileName(fileName)
    , m_running(false)
{
    qCDebug(lcUploader) << "Created upload. Filename:" << m_fileName
                        << "message id:" << messageId << "slot id:" << slotId;
}

VSQUpload::~VSQUpload()
{
    // FIXME(fpohtmeh): remove encrypted local file on success
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Upload";
#endif
}

QString VSQUpload::messageId() const
{
    return m_messageId;
}

QString VSQUpload::slotId() const
{
    return m_slotId;
}

void VSQUpload::setAttachment(const Attachment &attachment)
{
    const auto fileName = attachment.localUrl.toLocalFile();
    if (fileName != m_fileName) {
        qFatal("Attachment has different filename: %s -> %s", qPrintable(fileName), qPrintable(m_fileName));
    }
    m_attachment = attachment;
}

void VSQUpload::start()
{
    if (m_running) {
        qCWarning(lcUploader) << "Started again a running upload";
        return;
    }
    qCDebug(lcUploader) << QString("Started upload %1 / %2 / %3").arg(m_messageId, m_fileName, m_attachment.remoteUrl.toString());
    m_running = true;

    auto file = new QFile(m_attachment.filePath(), this);
    connect(this, &VSQUpload::finished, file, &QFile::deleteLater);
    connect(this, &VSQUpload::failed, file, &QFile::deleteLater);
    if (!file->open(QFile::ReadOnly)) {
        emit failed(QString("Unable to open for reading:").arg(m_attachment.filePath()));
        return;
    }

    // Create request
    QNetworkRequest request(m_attachment.remoteUrl);
    auto mimeType = QMimeDatabase().mimeTypeForUrl(m_attachment.encLocalUrl);
    if (mimeType.isValid())
        request.setHeader(QNetworkRequest::ContentTypeHeader, mimeType.name());
    request.setHeader(QNetworkRequest::ContentLengthHeader, m_attachment.bytesTotal);
    // Create & connect reply
    auto reply = m_networkAccessManager->put(request, file);
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "Created reply:" << reply;
#endif
    connect(reply, &QNetworkReply::uploadProgress, this, &VSQUpload::progressChanged);
    connect(reply, &QNetworkReply::finished, this, &VSQUpload::finished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, std::bind(&VSQUpload::onNetworkReplyError, this, args::_1, reply));
    connect(this, &VSQUpload::finished, reply, std::bind(&VSQUpload::cleanupReply, this, reply));
    connect(this, &VSQUpload::failed, reply, std::bind(&VSQUpload::cleanupReply, this, reply));
}

void VSQUpload::abort()
{
    if (!m_running) {
        qCWarning(lcUploader) << "Stopped not-running upload";
        return;
    }
    m_running = false;
    const QString message = QString("Aborted upload %1 / %2 / %3").arg(m_messageId, m_fileName, m_attachment.remoteUrl.toString());
    qCDebug(lcUploader) << message;
    emit failed(message);
}

void VSQUpload::onNetworkReplyError(QNetworkReply::NetworkError error, QNetworkReply *reply)
{
    emit failed(QString("Network error (code %1): %2").arg(error).arg(reply->errorString()));
}

void VSQUpload::cleanupReply(QNetworkReply *reply)
{
    m_running = false;
    reply->deleteLater();
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "Cleanuped reply:" << reply;
#endif
}
