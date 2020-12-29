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

#include "FileLoader.h"
#include "Utils.h"
#include "operations/MessageOperation.h"

using namespace vm;
using Self = UploadFileOperation;

Self::UploadFileOperation(NetworkOperation *parent, FileLoader *fileLoader, const QString &filePath)
    : LoadFileOperation(parent)
    , m_requestId(Utils::createUuid())
    , m_fileLoader(fileLoader)
{
    setName(QLatin1String("UploadFile"));
    setFilePath(filePath);
    connect(m_fileLoader, &FileLoader::uploadSlotRequestFinished, this, &Self::onSlotRequestFinished);
    connect(m_fileLoader, &FileLoader::uploadSlotRequestFailed, this, &Self::onSlotRequestFailed);
    connect(m_fileLoader, &FileLoader::uploadSlotReceived, this, &Self::onSlotUrlsReceived);
    connect(m_fileLoader, &FileLoader::uploadSlotErrorOccurred, this, &Self::onSlotUrlErrorOcurrend);
    connect(this, &Self::finished, this, &Self::onFinished);
}

void Self::run()
{
    if (!openFileHandle(QFile::ReadOnly)) {
        return;
    }

    m_fileLoader->requestUploadSlot(m_requestId, filePath());
}

void Self::connectReply(QNetworkReply *reply)
{
    LoadFileOperation::connectReply(reply);
    connect(reply, &QNetworkReply::uploadProgress, this, &LoadFileOperation::setProgress);
}

void Self::startUpload()
{
    m_fileLoader->startUpload(m_putUrl, fileHandle(), std::bind(&Self::connectReply, this, std::placeholders::_1));
}

void Self::onSlotRequestFinished(const QString &requestId, const QString &slotId)
{
    if (m_requestId == requestId) {
        m_slotId = slotId;
    }
}

void Self::onSlotRequestFailed(const QString &requestId)
{
    if (m_requestId == requestId) {
        qCWarning(lcOperation) << "Failed to request upload slot, file '" + filePath() + "' would not be uploaded.";
        fail();
    }
}

void Self::onSlotUrlsReceived(const QString &slotId, const QUrl &putUrl, const QUrl &getUrl)
{
    if (slotId == m_slotId) {
        qCDebug(lcOperation) << "Upload url received";
        m_putUrl = putUrl;
        m_getUrl = getUrl;
        emit uploadSlotReceived();
        startUpload();
    }
}

void Self::onSlotUrlErrorOcurrend(const QString &slotId, const QString &errorText)
{
    if (slotId == m_slotId) {
        if (status() != Status::Failed) {
            invalidate(errorText);
        }
    }
}

void Self::onFinished()
{
    qCDebug(lcOperation) << "File was uploaded";
    emit uploaded(m_getUrl);
}
