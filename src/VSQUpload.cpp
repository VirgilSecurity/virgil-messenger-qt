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

#include <QEventLoop>
#include <QMimeDatabase>
#include <QNetworkReply>
#include <QTimer>

VSQUpload::VSQUpload(QNetworkAccessManager *networkAccessManager, const QString &id, const QString &filePath, QObject *parent)
    : VSQTransfer(networkAccessManager, id, parent)
    , m_filePath(filePath)
{}

VSQUpload::~VSQUpload()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Upload" << m_filePath;
#endif
}

void VSQUpload::start()
{
    if (isRunning()) {
        qCWarning(lcTransferManager) << "Cannot start again a running upload";
        return;
    }
    qCDebug(lcTransferManager) << QString("Started upload: %1").arg(id());
    VSQTransfer::start();

    // Encrypt file
    auto file = createFileHandle(m_filePath);
    if (!file->open(QFile::ReadOnly)) {
        setStatus(Attachment::Status::Failed);
        return;
    }

    // Create request
    auto url = remoteUrl();
    if (!url) {
        setStatus(Attachment::Status::Failed);
        return;
    }
    QNetworkRequest request(*url);
    auto mimeType = QMimeDatabase().mimeTypeForUrl(m_filePath);
    if (mimeType.isValid()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, mimeType.name());
    }
    request.setHeader(QNetworkRequest::ContentLengthHeader, fileSize());
    // Create & connect reply
    auto reply = networkAccessManager()->put(request, file);
    connectReply(reply);
    connect(reply, &QNetworkReply::uploadProgress, [=](qint64 bytesSent, qint64 bytesTotal) {
        emit progressChanged(bytesSent, bytesTotal);
    });
}

QString VSQUpload::filePath() const
{
    return m_filePath;
}

Optional<QUrl> VSQUpload::remoteUrl()
{
    if (!m_remoteUrl && !m_remoteUrlError) {
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;

        auto remoteUrlError = [&]() {
            m_remoteUrlError = true;
            loop.quit();
        };

        connect(&timer, &QTimer::timeout, &loop, remoteUrlError);
        connect(this, &VSQUpload::remoteUrlReceived, &loop, [&](const QUrl &url) {
            m_remoteUrl = url;
            loop.quit();
        });
        connect(this, &VSQUpload::remoteUrlErrorOccured, &loop, remoteUrlError);
        connect(this, &VSQUpload::connectionChanged, &loop, remoteUrlError);

        timer.start(1000);
        qCDebug(lcTransferManager) << "Upload url waiting: start";
        loop.exec();
        qCDebug(lcTransferManager) << "Upload url waiting: end";
    }
    if (m_remoteUrlError) {
        qCDebug(lcTransferManager) << "Remote url error";
        return NullOptional;
    }
    if (m_remoteUrl) {
        qCDebug(lcTransferManager) << "Remote url:" << *m_remoteUrl;
        return m_remoteUrl;
    }
    return NullOptional;
}

QString VSQUpload::slotId() const
{
    return m_slotId;
}

void VSQUpload::setSlotId(const QString &id)
{
    m_slotId = id;
}

DataSize VSQUpload::fileSize() const
{
    return QFileInfo(filePath()).size();
}
