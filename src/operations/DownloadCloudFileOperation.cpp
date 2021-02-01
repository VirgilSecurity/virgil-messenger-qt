//  Copyright (C) 2015-2021 Virgil Security, Inc.
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

#include "DownloadCloudFileOperation.h"

#include "CloudFileOperation.h"
#include "CloudFileSystem.h"
#include "FileUtils.h"

using namespace vm;

DownloadCloudFileOperation::DownloadCloudFileOperation(CloudFileOperation *parent, const CloudFileHandler &file, const CloudFileHandler &parentFolder)
    : DownloadFileOperation(parent, parent->fileLoader(), QUrl(), file->size(), QString())
    , m_parent(parent)
    , m_file(file)
    , m_parentFolder(parentFolder)
    , m_requestId(0)
{
    setName(QLatin1String("DownloadCloudFile"));
    setFilePath(tempFilePath());

    connect(m_parent->cloudFileSystem(), &CloudFileSystem::downloadInfoGot, this, &DownloadCloudFileOperation::onDownloadInfoGot);
    connect(m_parent->cloudFileSystem(), &CloudFileSystem::getDownloadInfoErrorOccurred, this, &DownloadCloudFileOperation::onGetDownloadInfoErrorOccurred);
    connect(this, &DownloadFileOperation::progressChanged, this, &DownloadCloudFileOperation::onProgressChanged);
    connect(this, &DownloadFileOperation::downloaded, this, &DownloadCloudFileOperation::onDownloaded);
    connect(this, &DownloadFileOperation::failed, this, &DownloadCloudFileOperation::sendFailedTransferUpdate);
    connect(this, &DownloadFileOperation::invalidated, this, &DownloadCloudFileOperation::sendFailedTransferUpdate);
}

void DownloadCloudFileOperation::run()
{
    transferUpdate(TransferCloudFileUpdate::Stage::Started, 0);
    m_requestId = m_parent->cloudFileSystem()->getDownloadInfo(m_file);
}

void DownloadCloudFileOperation::cleanup()
{
    Operation::cleanup();
    FileUtils::removeFile(tempFilePath());
}

QString DownloadCloudFileOperation::tempFilePath() const
{
    return m_parent->settings()->cloudFilesCacheDir().filePath(QLatin1String("download-") + m_file->id());
}

void DownloadCloudFileOperation::onDownloadInfoGot(const CloudFileRequestId requestId, const CloudFileHandler &file, const QUrl &url, const QByteArray &encryptionKey)
{
    Q_UNUSED(file)
    if (m_requestId == requestId) {
        setUrl(url);
        m_encryptionKey = encryptionKey;
        qCDebug(lcOperation) << "Started to download cloud file from" << url;
        DownloadFileOperation::run();
    }
}

void DownloadCloudFileOperation::onGetDownloadInfoErrorOccurred(const CloudFileRequestId requestId, const QString &errorText)
{
    if(m_requestId == requestId) {
        failAndNotify(errorText);
    }
}

void DownloadCloudFileOperation::onProgressChanged(const quint64 bytesLoaded, const quint64 bytesTotal)
{
    Q_UNUSED(bytesTotal)
    transferUpdate(TransferCloudFileUpdate::Stage::Transfering, bytesLoaded);
}

void DownloadCloudFileOperation::onDownloaded()
{
    const auto decrypted = m_parent->cloudFileSystem()->decryptFile(tempFilePath(), m_encryptionKey, m_file);
    if (!decrypted) {
        failAndNotify(tr("File decryption was failed"));
    }

    // Send update
    DownloadCloudFileUpdate update;
    update.parentFolder = m_parentFolder;
    update.file = m_file;
    update.fingerprint = FileUtils::calculateFingerprint(m_file->localPath());
    m_parent->cloudFilesUpdate(update);

    transferUpdate(TransferCloudFileUpdate::Stage::Finished, m_file->size());

    finish();
}

void DownloadCloudFileOperation::sendFailedTransferUpdate()
{
    transferUpdate(TransferCloudFileUpdate::Stage::Failed, 0);
}

void DownloadCloudFileOperation::transferUpdate(const TransferCloudFileUpdate::Stage stage, const quint64 bytesLoaded)
{
    TransferCloudFileUpdate update;
    update.parentFolder = m_parentFolder;
    update.file = m_file;
    update.stage = stage;
    update.type = TransferCloudFileUpdate::Type::Download;
    update.bytesLoaded = bytesLoaded;
    m_parent->cloudFilesUpdate(update);
}
