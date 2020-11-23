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

#include "models/FileLoader.h"

#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QXmppClient.h>
#include <QXmppUploadRequestManager.h>

Q_LOGGING_CATEGORY(lcFileLoader, "fileLoader")

using namespace vm;

// TODO(fpohtmeh): re-design class slightly, methods are used from another thread

FileLoader::FileLoader(QXmppClient *client, QNetworkAccessManager *networkAccessManager, QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(networkAccessManager)
    , m_xmppManager(new QXmppUploadRequestManager())
{
    qRegisterMetaType<QXmppHttpUploadSlotIq>();
    qRegisterMetaType<QXmppHttpUploadRequestIq>();
    qRegisterMetaType<FileLoader::ConnectionSetup>("ConnectionSetup");

    m_xmppManager->setParent(this);
    client->addExtension(m_xmppManager);

    connect(m_xmppManager, &QXmppUploadRequestManager::serviceFoundChanged, this, &FileLoader::onServiceFound);
    connect(m_xmppManager, &QXmppUploadRequestManager::slotReceived, this, &FileLoader::onSlotReceived);
    connect(m_xmppManager, &QXmppUploadRequestManager::requestFailed, this, &FileLoader::onRequestFailed);

    connect(this, &FileLoader::startDownload, this, &FileLoader::onStartDownload);
    connect(this, &FileLoader::startUpload, this, &FileLoader::onStartUpload);
}

QString FileLoader::requestUploadUrl(const QString &filePath)
{
    if (m_serviceFound) {
        return m_xmppManager->requestUploadSlot(QFileInfo(filePath));
    }
    else {
        return QString();
    }
}

bool FileLoader::isServiceFound() const
{
    return m_serviceFound;
}

void FileLoader::onServiceFound()
{
    m_serviceFound = m_xmppManager->serviceFound();
    qCDebug(lcFileLoader) << "Upload service found:" << m_serviceFound;
    emit serviceFound(m_serviceFound);
    if (m_serviceFound) {
        emit ready();
    }
}

void FileLoader::onSlotReceived(const QXmppHttpUploadSlotIq &slot)
{
    emit slotUrlsReceived(slot.id(), slot.putUrl(), slot.getUrl());
}

void FileLoader::onRequestFailed(const QXmppHttpUploadRequestIq &request)
{
    const auto error = request.error();
    qCDebug(lcFileLoader) << QString("code(%1), condition(%2), text(%3)")
                             .arg(error.code()).arg(error.condition()).arg(error.text());
    if (error.condition() == QXmppStanza::Error::Condition::NotAcceptable && error.code() == 406) {
        emit slotUrlErrorOcurrend(request.id(), tr("File is larger than limit"));
    }
    else {
        emit slotUrlErrorOcurrend(request.id(), tr("File to upload file"));
    }
}

void FileLoader::onStartUpload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentLengthHeader, file->size());
    auto reply = m_networkAccessManager->put(request, file);
    connectionSetup(reply);
}

void FileLoader::onStartDownload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup)
{
    QNetworkRequest request(url);
    auto reply = m_networkAccessManager->get(request);
    connect(reply, &QNetworkReply::readyRead, [=]() {
        const auto bytes = reply->readAll();
        file->write(bytes);
        file->flush();
    });
    connectionSetup(reply);
}
