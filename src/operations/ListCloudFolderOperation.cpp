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

#include "ListCloudFolderOperation.h"

#include "CloudFileOperation.h"
#include "CloudFilesTable.h"
#include "CloudFileSystem.h"
#include "FileUtils.h"
#include "Messenger.h"
#include "UserDatabase.h"

using namespace vm;
using Self = vm::ListCloudFolderOperation;

Self::ListCloudFolderOperation(CloudFileOperation *parent, const CloudFileHandler &parentFolder,
                               UserDatabase *userDatabase)
    : Operation(QLatin1String("ListCloudFolder"), parent),
      m_parent(parent),
      m_parentFolder(parentFolder),
      m_userDatabase(userDatabase)
{
    connect(m_userDatabase->cloudFilesTable(), &CloudFilesTable::fetched, this, &Self::onDatabaseListFetched);
    connect(m_parent->cloudFileSystem(), &CloudFileSystem::listFetched, this, &Self::onCloudListFetched);
    connect(m_parent->cloudFileSystem(), &CloudFileSystem::fetchListErrorOccured, this,
            &Self::onCloudListFetchErrorOccurred);
}

void Self::run()
{
    m_userDatabase->cloudFilesTable()->fetch(m_parentFolder);
}

void ListCloudFolderOperation::onCloudListFetched(const CloudFileRequestId requestId,
                                                  const ModifiableCloudFileHandler &parentFolder,
                                                  const ModifiableCloudFiles &files)
{
    Q_UNUSED(parentFolder)
    if (m_requestId != requestId) {
        return;
    }

    m_parent->updateCloudFiles(buildDifference(parentFolder, files));
    deleteObsoleteLocalFiles(files);

    finish();
}

void ListCloudFolderOperation::onCloudListFetchErrorOccurred(CloudFileRequestId requestId, const QString &errorText)
{
    Q_UNUSED(errorText)
    if (m_requestId != requestId) {
        return;
    }
    fail();
}

CloudListCloudFolderUpdate ListCloudFolderOperation::buildDifference(const CloudFileHandler &parentFolder,
                                                                     const ModifiableCloudFiles &files) const
{
    CloudListCloudFolderUpdate update;
    update.parentFolder = parentFolder;
    auto oldFiles = m_cachedFiles;
    std::sort(std::begin(oldFiles), std::end(oldFiles), fileIdLess);
    auto newFiles = files;
    std::sort(std::begin(newFiles), std::end(newFiles), fileIdLess);
    const auto oldSize = oldFiles.size();
    const auto newSize = newFiles.size();
    size_t oldI = 0;
    size_t newI = 0;
    while (oldI < oldSize && newI < newSize) {
        const auto &oldFile = oldFiles[oldI];
        const auto &newFile = newFiles[newI];
        if (fileIdLess(oldFile, newFile)) {
            update.deleted.push_back(oldFile);
            ++oldI;
        } else if (fileIdLess(newFile, oldFile)) {
            update.added.push_back(newFile);
            ++newI;
        } else {
            if (fileUpdated(oldFile, newFile)) {
                update.updated.push_back(newFile);
            }
            ++oldI;
            ++newI;
        }
    }
    if (oldI < oldSize) {
        update.deleted.insert(update.deleted.end(), oldFiles.begin() + oldI, oldFiles.end());
    }
    if (newI < newSize) {
        update.added.insert(update.added.end(), newFiles.begin() + newI, newFiles.end());
    }
    return update;
}

void ListCloudFolderOperation::deleteObsoleteLocalFiles(const ModifiableCloudFiles &files)
{
    QSet<QString> filePaths;
    QSet<QString> folderPaths;
    for (auto &file : files) {
        (file->isFolder() ? folderPaths : filePaths) << file->localPath().toLower();
    }

    const auto localDir = QDir(m_parentFolder->localPath());
    const auto localFiles = localDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden);
    for (auto &localFile : localFiles) {
        const auto localPath = localFile.absoluteFilePath();
        if (localFile.isDir()) {
            if (!folderPaths.contains(localPath.toLower())) {
                FileUtils::removeDir(localPath);
            }
        } else if (localFile.isFile()) {
            if (!filePaths.contains(localPath.toLower())) {
                FileUtils::removeFile(localPath);
            }
        }
    }
}

void Self::onDatabaseListFetched(const CloudFileHandler &parentFolder, const ModifiableCloudFiles &cloudFiles)
{
    if (m_parentFolder->id() != parentFolder->id()) {
        return;
    }

    m_cachedFiles = cloudFiles;

    // Always update local paths
    const QDir parentDir(parentFolder->localPath());
    for (auto &cloudFile : m_cachedFiles) {
        cloudFile->setLocalPath(parentDir.filePath(cloudFile->name()));
    }

    CachedListCloudFolderUpdate update;
    update.parentFolder = parentFolder;
    update.files = m_cachedFiles;
    m_parent->updateCloudFiles(update);

    if (m_parent->messenger()->isOnline()) {
        m_requestId = m_parent->cloudFileSystem()->fetchList(m_parentFolder);
    } else {
        qCDebug(lcOperation) << "Network is offline";
        fail();
    }
}

bool Self::fileIdLess(const ModifiableCloudFileHandler &lhs, const ModifiableCloudFileHandler &rhs)
{
    return lhs->id() < rhs->id();
}

bool Self::fileUpdated(const ModifiableCloudFileHandler &lhs, const ModifiableCloudFileHandler &rhs)
{
    // NOTE(fpohtmeh): compare localPath because it changes after reinstall (IOS)
    // TODO(fpohtmeh): revert localPath comparision once we remove DB column for it
    return lhs->updatedAt() < rhs->updatedAt() || lhs->localPath() != rhs->localPath();
}
