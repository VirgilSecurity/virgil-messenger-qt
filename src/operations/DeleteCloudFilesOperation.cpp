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

#include "DeleteCloudFilesOperation.h"

#include "CloudFileOperation.h"
#include "Messenger.h"
#include "FileUtils.h"

using namespace vm;

DeleteCloudFilesOperation::DeleteCloudFilesOperation(CloudFileOperation *parent, const CloudFiles &files)
    : Operation(QLatin1String("DeleteCloudFiles"), parent),
      m_parent(parent),
      m_files(files),
      m_requestId(0),
      m_processedCount(0)
{
    auto fileSystem = m_parent->cloudFileSystem();
    connect(fileSystem, &CloudFileSystem::fileDeleted, this, &DeleteCloudFilesOperation::onFileDeleted);
    connect(fileSystem, &CloudFileSystem::deleteFileErrorOccurred, this,
            &DeleteCloudFilesOperation::onDeleteFileErrorOccured);
}

void DeleteCloudFilesOperation::run()
{
    m_requestId = m_parent->cloudFileSystem()->deleteFiles(m_files);
}

void DeleteCloudFilesOperation::incProcessedCount()
{
    if (++m_processedCount < m_files.size()) {
        return;
    }

    deleteLocalFiles();

    // Emit update
    DeleteCloudFilesUpdate update;
    update.files = m_deletedFiles;
    m_parent->cloudFilesUpdate(update);

    if (m_deletedFiles.size() < m_files.size()) {
        failAndNotify(tr("Failed to delete file"));
    } else {
        finish();
    }
}

void DeleteCloudFilesOperation::onFileDeleted(const CloudFileRequestId requestId, const CloudFileHandler &file)
{
    if (m_requestId == requestId) {
        m_deletedFiles.push_back(file);
        incProcessedCount();
    }
}

void DeleteCloudFilesOperation::onDeleteFileErrorOccured(const CloudFileRequestId requestId, const QString &errorText)
{
    Q_UNUSED(errorText)
    if (m_requestId == requestId) {
        incProcessedCount();
    }
}

void DeleteCloudFilesOperation::deleteLocalFiles()
{
    for (auto &file : m_deletedFiles) {
        const QFileInfo fileInfo(file->localPath());
        if (fileInfo.isDir()) {
            FileUtils::removeDir(file->localPath());
        } else if (fileInfo.isFile()) {
            FileUtils::removeFile(file->localPath());
        }
    }
}
