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

#include "operations/DownloadFileOperation.h"

#include <QNetworkReply>

#include "FileLoader.h"
#include "operations/MessageOperation.h"

using namespace vm;

DownloadFileOperation::DownloadFileOperation(NetworkOperation *parent, FileLoader *fileLoader, const QUrl &url, const DataSize &bytesTotal, const QString &filePath)
    : LoadFileOperation(parent, bytesTotal)
    , m_url(url)
    , m_fileLoader(fileLoader)
{
    setName(QLatin1String("DownloadFile"));
    if (bytesTotal <= 0) {
        qCWarning(lcOperation) << "Download file operation can't determine bytesTotal. Pass correct bytesTotal to constructor";
    }
    setFilePath(filePath);
    connect(this, &DownloadFileOperation::finished, this, &DownloadFileOperation::onFinished);
}

void DownloadFileOperation::run()
{
    if (!openFileHandle(QFile::WriteOnly)) {
        return;
    }
    m_fileLoader->fireStartDownload(m_url, fileHandle(), std::bind(&DownloadFileOperation::connectReply, this, args::_1));
}

void DownloadFileOperation::connectReply(QNetworkReply *reply)
{
    LoadFileOperation::connectReply(reply);
    connect(reply, &QNetworkReply::downloadProgress, this, &LoadFileOperation::setProgress);
}

void DownloadFileOperation::onFinished()
{
    qCDebug(lcOperation) << "File was downloaded to:" << filePath();
    emit downloaded(filePath());
}
