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
#include "FileUtils.h"
#include "operations/MessageOperation.h"

using namespace vm;

LoadFileOperation::LoadFileOperation(NetworkOperation *parent, quint64 bytesTotal)
    : NetworkOperation(parent)
    , m_bytesTotal(bytesTotal)
{
    setName(QLatin1String("LoadFile"));
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
    connect(reply, &QNetworkReply::finished, this, std::bind(&LoadFileOperation::onReplyFinished, this, reply));
    connect(reply, &QNetworkReply::errorOccurred, this, std::bind(&LoadFileOperation::onReplyErrorOccurred, this, std::placeholders::_1, reply));
    connect(reply, &QNetworkReply::sslErrors, this, &LoadFileOperation::onReplySslErrors);
    connect(this, &LoadFileOperation::interrupt, reply, &QNetworkReply::abort);
}

bool LoadFileOperation::openFileHandle(const QIODevice::OpenMode &mode)
{
    if (m_filePath.isEmpty()) {
        qCWarning(lcOperation) << "File path is empty";
        invalidateAndNotify(tr("File path is empty"));
        return false;
    }

    if ((mode == QFile::ReadOnly) && !FileUtils::fileExists(m_filePath)) {
        qCWarning(lcOperation) << "File doesn't exist" << m_filePath;
        invalidateAndNotify(tr("File doesn't exist"));
        return false;
    }

    m_fileHandle.reset(new QFile(m_filePath));
    if (!m_fileHandle->open(mode)) {
        qCWarning(lcOperation) << "File can't be opened" << m_filePath;
        invalidateAndNotify(tr("File can't be opened"));
        return false;
    }

    qCDebug(lcOperation) << "File handle was opened:" << mode << m_filePath;
    return true;
}

void LoadFileOperation::closeFileHandle()
{
    if (m_fileHandle) {
        m_fileHandle.reset();
        qCDebug(lcOperation) << "File handle was closed:" << m_filePath;
    }
}

QFile *LoadFileOperation::fileHandle()
{
    return &*m_fileHandle;
}

QString LoadFileOperation::filePath() const
{
    return m_filePath;
}

void LoadFileOperation::onReplyFinished(QNetworkReply *reply)
{
    const auto error = reply->error();
    reply->deleteLater();
    if (status() == Status::Failed) {
        return;
    }
    qCDebug(lcOperation) << "Reply finished" << Utils::printableLoadProgress(m_bytesLoaded, m_bytesTotal);
    closeFileHandle();
    if (m_bytesTotal > 0 && m_bytesLoaded >= m_bytesTotal) {
        qCDebug(lcOperation) << "Reply success";
        finish();
    }
    else {
        qCWarning(lcOperation) << "Failed. Load file was processed partially" << error;
        invalidateAndNotify(tr("File loading failed"));
    }
}

void LoadFileOperation::onReplyErrorOccurred(const QNetworkReply::NetworkError error, QNetworkReply *reply)
{
    Q_UNUSED(reply)
    if (status() == Status::Failed) {
        return;
    }
    using Error = QNetworkReply::NetworkError;
    switch (error) {
    case Error::TemporaryNetworkFailureError:
    case Error::NetworkSessionFailedError:
    case Error::UnknownNetworkError:
        qCDebug(lcOperation) << "Failed due to temporary network issue";
        fail();
        break;
    case Error::OperationCanceledError:
        qCWarning(lcOperation) << "File loading cancelled";
        fail();
        emit notificationCreated(tr("File loading cancelled"), false);
        break;
    default:
        qCWarning(lcOperation) << "File load error occurred:" << error;
        invalidateAndNotify(tr("File loading error: %1").arg(error));
        break;
    }
}

void LoadFileOperation::onReplySslErrors()
{
    qCWarning(lcOperation) << "SSL errors occurred";
}

void LoadFileOperation::onSetProgress(quint64 bytesLoaded, quint64 bytesTotal)
{
    if (bytesTotal == 0 && bytesLoaded == 0) {
        // NOTE(fpohtmeh): Qt finishes upload with zero values
        return;
    }
    else if (bytesTotal == quint64(-1)) {
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
    //qCDebug(lcOperation) << "Load progress:" << Utils::printableLoadProgress(m_bytesLoaded, m_bytesTotal);
    emit progressChanged(m_bytesLoaded, m_bytesTotal);
}
