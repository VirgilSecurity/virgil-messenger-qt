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

VSQUpload::VSQUpload(QNetworkAccessManager *networkAccessManager, const QString &messageId, const QString &slotId, QObject *parent)
    : VSQTransfer(networkAccessManager, messageId, parent)
    , m_slotId(slotId)
{}

VSQUpload::~VSQUpload()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Upload" << messageId();
#endif
}

QString VSQUpload::slotId() const
{
    return m_slotId;
}

void VSQUpload::start()
{
    if (m_running) {
        qCWarning(lcTransferManager) << "Cannot start again a running upload";
        return;
    }
    qCDebug(lcTransferManager) << QString("Started upload %1 / %2 / %3")
                                  .arg(messageId(), m_attachment.filePath(), m_attachment.remoteUrl.toString());
    m_running = true;

    // Check file for reading
    auto file = getAttachmentFile();
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
    connectReply(reply);
}
