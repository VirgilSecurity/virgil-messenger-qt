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

#include "operations/UploadFileOperation.h"

#include <QFile>
#include <QNetworkReply>

#include "models/FileLoader.h"
#include "operations/MessageOperation.h"

using namespace vm;

UploadFileOperation::UploadFileOperation(const QString &name, QObject *parent, const QString &filePath, FileLoader *fileLoader)
    : LoadFileOperation(name, parent, fileLoader)
{
    setFilePath(filePath);
    connect(fileLoader, &FileLoader::slotUrlsReceived, this, &UploadFileOperation::onSlotUrlsReceived);
    connect(fileLoader, &FileLoader::slotUrlErrorOcurrend, this, &UploadFileOperation::onSlotUrlErrorOcurrend);
    connect(this, &UploadFileOperation::finished, this, &UploadFileOperation::onFinished);
}

void UploadFileOperation::run()
{
    if (!openFileHandle(QFile::ReadOnly)) {
        return;
    }
    m_slotId = fileLoader()->requestUploadUrl(filePath());
    if (m_slotId.isEmpty()) {
        qCWarning(lcOperation) << "Unable to request upload url";
        fail();
    }
}

void UploadFileOperation::cleanup()
{
    closeFileHandle();
    LoadFileOperation::cleanup();
}

void UploadFileOperation::connectReply(QNetworkReply *reply)
{
    LoadFileOperation::connectReply(reply);
    connect(reply, &QNetworkReply::uploadProgress, this, &LoadFileOperation::setProgress);
}

void UploadFileOperation::startUpload()
{
    fileLoader()->startUpload(m_putUrl, fileHandle(), std::bind(&UploadFileOperation::connectReply, this, args::_1));
}

void UploadFileOperation::onSlotUrlsReceived(const QString &slotId, const QUrl &putUrl, const QUrl &getUrl)
{
    if (slotId == m_slotId) {
        qCDebug(lcOperation) << "Upload url received";
        m_putUrl = putUrl;
        m_getUrl = getUrl;
        startUpload();
    }
}

void UploadFileOperation::onSlotUrlErrorOcurrend(const QString &slotId, const QString &errorText)
{
    if (slotId == m_slotId) {
        qCDebug(lcOperation) << "Unable to get upload url. Error:" << errorText;
        if (isConnectionChanged()) {
            fail();
        }
        else {
            emit notificationCreated(tr("Unable to upload file"));
            invalidate();
        }
    }
}

void UploadFileOperation::onFinished()
{
    qCDebug(lcOperation) << "File was uploaded";
    emit uploaded(m_getUrl);
}
