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

VSQDownload::VSQDownload(QNetworkAccessManager *networkAccessManager, const QString &id,
                         const QUrl &remoteUrl, const QString &filePath, QObject *parent)
    : VSQTransfer(networkAccessManager, id, parent)
    , m_remoteUrl(remoteUrl)
    , m_filePath(filePath)
{}

VSQDownload::~VSQDownload()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Download" << m_filePath;
#endif
}

void VSQDownload::start()
{
    if (isRunning()) {
        qCWarning(lcTransferManager) << "Cannot start again a running download";
        return;
    }
    qCDebug(lcTransferManager) << QString("Started download: %1").arg(id());
    VSQTransfer::start();

    // Check file for writing
    auto file = createFileHandle(m_filePath);
    if (!file->open(QFile::WriteOnly)) {
        setStatus(Attachment::Status::Failed);
        return;
    }

    // Create request
    QNetworkRequest request(m_remoteUrl);
    auto reply = networkAccessManager()->get(request);
    connectReply(reply, &m_guard);
    connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
        emit progressChanged(bytesReceived, bytesTotal);
    });
    connect(reply, &QNetworkReply::readyRead, [=]() {
        const auto bytes = reply->readAll();
        //qCDebug(lcTransferManager()) << "Wrote bytes:" << bytes.size();
        file->write(bytes);
        file->flush();
    });
}
