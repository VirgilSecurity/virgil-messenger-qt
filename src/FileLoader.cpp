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

#include "FileLoader.h"

#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

Q_LOGGING_CATEGORY(lcFileLoader, "fileLoader");

using namespace vm;
using Self = FileLoader;

Self::FileLoader(CoreMessenger *commKitMessenger, QObject *parent)
    : QObject(parent), m_coreMessenger(commKitMessenger), m_networkAccessManager(new QNetworkAccessManager(this))
{

    qRegisterMetaType<Self::ConnectionSetup>("ConnectionSetup");

    connect(m_coreMessenger, &CoreMessenger::uploadServiceFound, this, &Self::uploadServiceFound);
    connect(m_coreMessenger, &CoreMessenger::uploadSlotReceived, this, &Self::uploadSlotReceived);
    connect(m_coreMessenger, &CoreMessenger::uploadSlotErrorOccurred, this, &Self::uploadSlotErrorOccurred);

    connect(this, &Self::startDownload, this, &Self::onStartDownload);
    connect(this, &Self::startUpload, this, &Self::onStartUpload);
    connect(this, &Self::requestUploadSlot, this, &Self::onRequestUploadSlot);
}

bool Self::isServiceFound() const
{
    return m_coreMessenger->isUploadServiceFound();
}

void Self::onRequestUploadSlot(const QString &requestId, const QString &filePath)
{
    const auto slotId = m_coreMessenger->requestUploadSlot(filePath);

    if (slotId.isEmpty()) {
        emit uploadSlotRequestFailed(requestId);
    } else {
        emit uploadSlotRequestFinished(requestId, slotId);
    }
}

void Self::onStartUpload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup)
{

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentLengthHeader, file->size());
    auto reply = m_networkAccessManager->put(request, file);
    connectionSetup(reply);
}

void Self::onStartDownload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup)
{

    QNetworkRequest request(url);
    auto reply = m_networkAccessManager->get(request);
    connect(reply, &QNetworkReply::readyRead, [file, reply]() {
        const auto bytes = reply->readAll();
        file->write(bytes);
        file->flush();
    });
    connectionSetup(reply);
}
