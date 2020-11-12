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
#include "models/FileLoader.h"
#include "operations/MessageOperation.h"

using namespace vm;

LoadFileOperation::LoadFileOperation(const QString &name, QObject *parent, FileLoader *fileLoader, const DataSize &bytesTotal)
    : Operation(name, parent)
    , m_fileLoader(fileLoader)
    , m_bytesTotal(bytesTotal)
{
    connect(this, &LoadFileOperation::setProgress, this, &LoadFileOperation::onSetProgress);
    connect(fileLoader, &FileLoader::serviceFound, this, &LoadFileOperation::setConnectionChanged);
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
    connect(reply, &QNetworkReply::errorOccurred, this, std::bind(&LoadFileOperation::onReplyErrorOccurred, this, args::_1, reply));
    connect(reply, &QNetworkReply::sslErrors, this, &LoadFileOperation::onReplySslErrors);
}

void LoadFileOperation::cleanup()
{
    Operation::cleanup();
    m_isConnectionChanged = false;
}

bool LoadFileOperation::openFileHandle(const QIODevice::OpenMode &mode)
{
    if (m_filePath.isEmpty()) {
        qCWarning(lcOperation) << "File path is empty";
        emit notificationCreated(tr("File path is empty"));
        invalidate();
        return false;
    }

    if ((mode == QFile::ReadOnly) && !Utils::fileExists(m_filePath)) {
        qCWarning(lcOperation) << "File doesn't exist" << m_filePath;
        emit notificationCreated(tr("File doesn't exist"));
        invalidate();
        return false;
    }

    m_fileHandle.reset(new QFile(m_filePath));
    if (!m_fileHandle->open(mode)) {
        qCWarning(lcOperation) << "File can't be opened" << m_filePath;
        emit notificationCreated(tr("File can't be opened"));
        invalidate();
        return false;
    }

    qCDebug(lcOperation) << "File handle was opened:" << mode << Utils::fileName(m_filePath);
    return true;
}

void LoadFileOperation::closeFileHandle()
{
    if (m_fileHandle) {
        m_fileHandle.reset();
        qCDebug(lcOperation) << "File handle was closed:" << Utils::fileName(m_filePath);
    }
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

bool LoadFileOperation::isConnectionChanged() const
{
    return m_isConnectionChanged;
}

void LoadFileOperation::onReplyFinished()
{
    qCDebug(lcOperation) << "Reply finished" << Utils::printableLoadProgress(m_bytesLoaded, m_bytesTotal);
    closeFileHandle();
    if (m_bytesTotal > 0 && m_bytesLoaded >= m_bytesTotal) {
        qCDebug(lcOperation) << "Reply success";
        finish();
    }
    else {
        qCWarning(lcOperation) << "Failed. Load file was processed partially";
        if (m_isConnectionChanged) {
            fail();
        }
        else {
            emit notificationCreated(tr("File loading failed"));
            invalidate();
        }
    }
}

void LoadFileOperation::onReplyErrorOccurred(const QNetworkReply::NetworkError &error, QNetworkReply *)
{
    qCWarning(lcOperation) << "File load error occurred:" << error << static_cast<int>(error);
    if (m_isConnectionChanged) {
        fail();
    }
    else {
        emit notificationCreated(tr("File loading error: %1").arg(error));
        invalidate();
    }
}

void LoadFileOperation::onReplySslErrors()
{
    qCWarning(lcOperation) << "SSL errors occurred";
}

void LoadFileOperation::onSetProgress(const DataSize &bytesLoaded, const DataSize &bytesTotal)
{
    if (bytesTotal == 0 && bytesLoaded == 0) {
        // NOTE(fpohtmeh): Qt finishes upload with zero values
        return;
    }
    else if (bytesTotal == -1) {
        // NOTE(fpohtmeh): Qt uses correct bytesLoaded and bytesTotal=-1 when starts download
        // and finishes it with correct bytesTotal and bytesLoaded=bytesTotal
        if (m_bytesTotal < bytesLoaded) {
            qCWarning(lcOperation) << "Pass bytesTotal to constructor for correct progress calculation";
            m_bytesTotal = bytesLoaded;
        }
    }
    else {
        m_bytesTotal = bytesTotal;
    }
    m_bytesLoaded = bytesLoaded;
    qCDebug(lcOperation) << "Load progress:" << Utils::printableLoadProgress(m_bytesLoaded, m_bytesTotal);
    emit progressChanged(m_bytesLoaded, m_bytesTotal);
}

void LoadFileOperation::setConnectionChanged()
{
    if (status() == Operation::Status::Started) {
        qCDebug(lcOperation) << "Connection was chnaged";
        m_isConnectionChanged = true;
    }
}
