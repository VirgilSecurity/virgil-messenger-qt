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

#include "operations/LoadFileOperation.h"

#include "Utils.h"
#include "operations/MessageOperation.h"

using namespace vm;

LoadFileOperation::LoadFileOperation(const QString &name, QObject *parent, FileLoader *fileLoader)
    : Operation(name, parent)
    , m_fileLoader(fileLoader)
{
    connect(this, &LoadFileOperation::setProgress, this, &LoadFileOperation::onSetProgress);
}

void LoadFileOperation::setFilePath(const QString &filePath)
{
    if (m_filePath == filePath) {
        return;
    }
    m_filePath = filePath;
    closeFileHandle();
}

void LoadFileOperation::connectReply(QNetworkReply *reply)
{
    connect(reply, &QNetworkReply::finished, this, &LoadFileOperation::onReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred, this, &LoadFileOperation::onReplyErrorOccurred);
    connect(reply, &QNetworkReply::sslErrors, this, &LoadFileOperation::onReplySslErrors);
}

bool LoadFileOperation::openFileHandle(const QIODevice::OpenMode &mode)
{
    qCDebug(lcOperation) << "Opening of file handle:" << mode << m_filePath;
    if (m_filePath.isEmpty()) {
        qCWarning(lcOperation) << "File path is empty";
        invalidate();
        return false;
    }

    if (mode == QFile::ReadOnly && !Utils::fileExists(m_filePath)) {
        qCWarning(lcOperation) << "File doesn't exist" << m_filePath;
        invalidate();
        return false;
    }

    m_fileHandle.reset(new QFile(m_filePath));
    if (!m_fileHandle->open(mode)) {
        qCWarning(lcOperation) << "File can't be opened" << m_filePath;
        invalidate();
        return false;
    }

    qCDebug(lcOperation) << "File handle was opened:" << mode << m_filePath;
    return true;
}

void LoadFileOperation::closeFileHandle()
{
    m_fileHandle.reset();
}

QFile *LoadFileOperation::fileHandle()
{
    return &*m_fileHandle;
}

FileLoader *LoadFileOperation::fileLoader()
{
    return m_fileLoader;
}

QString LoadFileOperation::filePath() const
{
    return m_filePath;
}

void LoadFileOperation::onReplyFinished()
{
    if (m_bytesTotal > 0 && m_bytesLoaded >= m_bytesTotal) {
        qCDebug(lcOperation) << "Reply finished";
        closeFileHandle();
        finish();
    }
    else {
        qCWarning(lcOperation) << "Failed. Load file was processed partially";
        fail();
    }
}

void LoadFileOperation::onReplyErrorOccurred(const QNetworkReply::NetworkError &error)
{
    qCWarning(lcOperation) << "File load error occurred:" << error;
    fail();
}

void LoadFileOperation::onReplySslErrors()
{
    qCWarning(lcOperation) << "SSL errors occurred";
}

void LoadFileOperation::onSetProgress(const DataSize &bytesLoaded, const DataSize &bytesTotal)
{
    //qCDebug(lcOperation) << "Load progress:" << Utils::printableLoadProgress(bytesLoaded, total);
    m_bytesLoaded = bytesLoaded;
    m_bytesTotal = bytesTotal;
    if (bytesTotal < 0) {
        // NOTE(fpohtmeh): download reply sends zeros as bytesTotal
        emit progressChanged(bytesLoaded, bytesLoaded);
    }
    else if (bytesLoaded < bytesTotal) {
        emit progressChanged(bytesLoaded, bytesTotal);
    }
    else {
        qCDebug(lcOperation) << "All bytes were processed, set load operation finished";
        closeFileHandle();
        finish();
    }
}
