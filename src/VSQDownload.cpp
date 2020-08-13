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

#include "VSQDownload.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include "VSQUtils.h"

VSQDownload::VSQDownload(QNetworkAccessManager *networkAccessManager, const QString &messageId,
                         const QDir &downloadDir, QObject *parent)
    : VSQTransfer(networkAccessManager, messageId, parent)
    , m_downloadDir(downloadDir)
{}

VSQDownload::~VSQDownload()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Download" << messageId();
#endif
}

void VSQDownload::start()
{
    if (m_running) {
        qCWarning(lcTransferManager) << "Cannot start again a running download";
        return;
    }
    qCDebug(lcTransferManager) << QString("Started download %1 / %2 / %3")
                                  .arg(messageId(), m_attachment.filePath(), m_attachment.remoteUrl.toString());

    // Check file for writing
    auto file = getAttachmentFile();
    if (!file->open(QFile::WriteOnly)) {
        emit failed(QString("Unable to open for writing:").arg(m_attachment.localUrl.toString()));
        return;
    }

    // Create request
    QNetworkRequest request(m_attachment.remoteUrl);
    auto reply = m_networkAccessManager->get(request);
    connectReply(reply);
    connect(reply, &QNetworkReply::readyRead, this, [=]() {
        file->write(reply->readAll());
    });
}

void VSQDownload::setAttachment(const Attachment &attachment)
{
    VSQTransfer::setAttachment(attachment);
    auto encFilePath = m_downloadDir.filePath(VSQUtils::createUuid()) + QLatin1Char('.') + m_attachment.fileExtension();
#ifdef VS_DEVMODE
    qCDebug(lcTransferManager) << "Encrypted file path:" << encFilePath;
#endif
    m_attachment.encLocalUrl = QUrl::fromLocalFile(encFilePath);
}
