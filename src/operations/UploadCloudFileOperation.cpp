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

#include "UploadCloudFileOperation.h"

#include <QDir>
#include <QFile>
#include <QMimeDatabase>

#include "CloudFileOperation.h"
#include "FileUtils.h"
#include "Messenger.h"
#include "Utils.h"

using namespace vm;

UploadCloudFileOperation::UploadCloudFileOperation(CloudFileOperation *parent, const QString &filePath, const CloudFileHandler &parentFolder)
    : UploadFileOperation(parent, parent->fileLoader(), QString()) // filePath will be known after encryption
    , m_parent(parent)
    , m_parentFolder(parentFolder)
    , m_requestId(0)
    , m_sourceFilePath(filePath)
{
    setName(QLatin1String("UploadCloudFile"));

    connect(m_parent->cloudFileSystem(), &CloudFileSystem::fileCreated, this, &UploadCloudFileOperation::onFileCreated);
    connect(m_parent->cloudFileSystem(), &CloudFileSystem::createFileErrorOccurred, this, &UploadCloudFileOperation::onCreateCloudFileErrorOccurred);
    connect(this, &UploadFileOperation::progressChanged, this, &UploadCloudFileOperation::onProgressChanged);
    connect(this, &UploadFileOperation::uploaded, this, &UploadCloudFileOperation::onUploaded);
    connect(this, &UploadFileOperation::failed, this, &UploadCloudFileOperation::sendFailedTransferUpdate);
    connect(this, &UploadFileOperation::invalidated, this, &UploadCloudFileOperation::sendFailedTransferUpdate);
}

void UploadCloudFileOperation::run()
{
    m_requestId = m_parent->cloudFileSystem()->createFile(m_sourceFilePath, m_parentFolder);
}

void UploadCloudFileOperation::cleanup()
{
    Operation::cleanup();
    FileUtils::removeFile(filePath()); // remove encrypted file
}

void UploadCloudFileOperation::onFileCreated(const CloudFileRequestId requestId, const ModifiableCloudFileHandler &cloudFile, const QString &encryptedFilePath, const QUrl &putUrl)
{
    if (m_requestId != requestId) {
        return;
    }

    setFilePath(encryptedFilePath);
    if (!openFileHandle(QFile::ReadOnly)) {
        return;
    }

    m_file = cloudFile;
    transferUpdate(TransferCloudFileUpdate::Stage::Started, 0);
    qCDebug(lcOperation) << "Started to upload cloud file to" << putUrl;
    startUploadToSlot(putUrl, putUrl); // NOTE(fpohtmeh): We don't know get url at this moment, using of putUrl is fine
}

void UploadCloudFileOperation::onCreateCloudFileErrorOccurred(const CloudFileRequestId requestId, const QString &errorText)
{
    if (m_requestId == requestId) {
        failAndNotify(errorText);
    }
}

void UploadCloudFileOperation::onProgressChanged(const quint64 bytesLoaded, const quint64 bytesTotal)
{
    Q_UNUSED(bytesTotal)
    transferUpdate(TransferCloudFileUpdate::Stage::Transfering, bytesLoaded);
}

void UploadCloudFileOperation::onUploaded()
{
    // Copy file to cloud downloads dir
    const QDir parentFolderDir(m_parentFolder->localPath());
    const auto cloudFileLocalPath = parentFolderDir.filePath(m_file->name());
    if (parentFolderDir.mkpath(".")) {
        QFile::copy(m_sourceFilePath, cloudFileLocalPath);
        m_file->setLocalPath(cloudFileLocalPath);
        m_file->setFingerprint(FileUtils::calculateFingerprint(cloudFileLocalPath));
    }

    // Send updates
    CreateCloudFilesUpdate update;
    update.parentFolder = m_parentFolder;
    update.files.push_back(m_file);
    m_parent->cloudFilesUpdate(update);

    transferUpdate(TransferCloudFileUpdate::Stage::Finished, m_file->size());

    finish();
}

void UploadCloudFileOperation::sendFailedTransferUpdate()
{
    transferUpdate(TransferCloudFileUpdate::Stage::Failed, 0);
}

void UploadCloudFileOperation::transferUpdate(const TransferCloudFileUpdate::Stage stage, const quint64 bytesLoaded)
{
    if (m_file) {
        TransferCloudFileUpdate update;
        update.parentFolder = m_parentFolder;
        update.file = m_file;
        update.stage = stage;
        update.type = TransferCloudFileUpdate::Type::Upload;
        update.bytesLoaded = bytesLoaded;
        m_parent->cloudFilesUpdate(update);
    }
}
