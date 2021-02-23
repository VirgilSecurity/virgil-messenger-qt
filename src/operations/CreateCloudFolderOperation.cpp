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

#include "CreateCloudFolderOperation.h"

#include "CloudFileOperation.h"
#include "CloudFilesQueueListeners.h"
#include "FileUtils.h"
#include "Messenger.h"

using namespace vm;

CreateCloudFolderOperation::CreateCloudFolderOperation(CloudFileOperation *parent, const QString &name,
                                                       const CloudFileHandler &parentFolder)
    : Operation(QLatin1String("CreateCloudFolder"), parent),
      m_parent(parent),
      m_name(name),
      m_parentFolder(parentFolder),
      m_requestId(0)
{
    connect(m_parent->cloudFileSystem(), &CloudFileSystem::folderCreated, this, &CreateCloudFolderOperation::onCreated);
    connect(m_parent->cloudFileSystem(), &CloudFileSystem::createFolderErrorOccured, this,
            &CreateCloudFolderOperation::onCreateErrorOccured);
}

void CreateCloudFolderOperation::run()
{
    // Local folder check
    const auto localPath = QDir(m_parentFolder->localPath()).filePath(m_name);
    if (FileUtils::fileExists(localPath)) {
        qCDebug(lcOperation) << "Folder name is not unique:" << localPath;
        failAndNotify(tr("Folder name is not unique"));
        return;
    }

    m_parent->watchFolderAndRun(m_parentFolder, this, [this](auto folder) {
        m_parentFolder = folder;
        m_requestId = m_parent->cloudFileSystem()->createFolder(m_name, m_parentFolder);
    });
}

void CreateCloudFolderOperation::onCreated(const CloudFileRequestId requestId, const ModifiableCloudFileHandler &folder)
{
    if (m_requestId == requestId) {
        FileUtils::forceCreateDir(folder->localPath());

        CreateCloudFilesUpdate update;
        update.parentFolder = m_parentFolder;
        update.files.push_back(folder);
        m_parent->cloudFilesUpdate(update);
        finish();
    }
}

void CreateCloudFolderOperation::onCreateErrorOccured(const CloudFileRequestId requestId, const QString &errorText)
{
    if (m_requestId == requestId) {
        failAndNotify(errorText);
    }
}
