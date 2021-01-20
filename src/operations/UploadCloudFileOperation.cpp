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
#include "Utils.h"

using namespace vm;

UploadCloudFileOperation::UploadCloudFileOperation(CloudFileOperation *parent, const QString &filePath, const CloudFileHandler &folder)
    : Operation(QLatin1String("UploadCloudFile"), parent)
    , m_parent(parent)
    , m_filePath(filePath)
    , m_folder(folder)
{
}

void UploadCloudFileOperation::run()
{
    // Create local dir & copy file to local dir
    QDir folderDir(m_folder->localPath());
    folderDir.mkpath(".");
    QFileInfo info(m_filePath);
    const auto localPath = folderDir.filePath(info.fileName());
    QFile::copy(info.absoluteFilePath(), localPath);

    // Create cloud file
    auto cloudFile = std::make_shared<CloudFile>();
    cloudFile->setId(Utils::createUuid());
    cloudFile->setParentId(m_folder->id());
    cloudFile->setName(info.fileName());
    cloudFile->setIsFolder(false);
    cloudFile->setType(FileUtils::fileMimeType(m_filePath));
    cloudFile->setSize(info.size());
    const auto now = QDateTime::currentDateTime();
    cloudFile->setCreatedAt(now);
    cloudFile->setUpdatedAt(now);
    cloudFile->setUpdatedBy(m_parent->userId());
    cloudFile->setLocalPath(localPath);
    cloudFile->setFingerprint(FileUtils::calculateFingerprint(m_filePath));

    // Send update
    CreatedCloudFileUpdate update;
    update.cloudFileId = cloudFile->id();
    update.cloudFile = cloudFile;
    m_parent->cloudFileUpdate(update);

    finish();
}
