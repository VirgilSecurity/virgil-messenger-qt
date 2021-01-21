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

#include "CloudFileSystem.h"

#include <QMimeDatabase>

#include "CoreMessenger.h"
#include "FutureWorker.h"

Q_LOGGING_CATEGORY(lcCloudFileSystem, "cloud-fs")

using namespace vm;

CloudFileSystem::CloudFileSystem(CoreMessenger *coreMessenger, QObject *parent)
    : QObject(parent)
    , m_coreMessenger(coreMessenger)
{
}

void CloudFileSystem::signIn()
{
    qCDebug(lcCloudFileSystem) << "Sign-in cloud-fs";
    m_coreFs = m_coreMessenger->cloudFs();
}

void CloudFileSystem::signOut()
{
    qCDebug(lcCloudFileSystem) << "Sign-out cloud-fs";
    m_coreFs = {};
}

void CloudFileSystem::fetchList(const CloudFileHandler &parentFolder)
{
    const auto parentId = parentFolder->id().coreFolderId();
    const auto isRoot = !parentId.isValid();
    FutureWorker::run(m_coreFs->listFolder(parentId), [this, parentFolder, isRoot](auto result) {
        if (std::holds_alternative<CoreMessengerStatus>(result)) {
            emit fetchListErrorOccured(tr("Cloud folder listing error"));
            return;
        }
        const auto fsFolder = std::get_if<CloudFsFolder>(&result);
        const auto parentFolderId = fsFolder->info.id;
        // Update fetched folder
        auto listedFolder = createFolderFromInfo(fsFolder->info, parentFolderId);
        if (isRoot) {
            listedFolder->setId(CloudFileId()); // empty id for root
        }
        listedFolder->setEncryptedKey(fsFolder->folderEncryptedKey);
        listedFolder->setPublicKey(fsFolder->folderPublicKey);
        listedFolder->setLocalPath(parentFolder->localPath());
        const QDir listedDir(parentFolder->localPath());
        // Build list
        ModifiableCloudFiles list;
        for (auto &info : fsFolder->folders) {
            auto folder = createFolderFromInfo(info, parentFolderId);
            folder->setLocalPath(listedDir.filePath(info.name));
            list.push_back(folder);
        }
        for (auto &info : fsFolder->files) {
            auto file = createFileFromInfo(info, parentFolderId);
            file->setLocalPath(listedDir.filePath(info.name));
            list.push_back(file);
        }
        emit listFetched(listedFolder, list);
    });
}

void CloudFileSystem::createFolder(const QString &name, const CloudFileHandler &parentFolder)
{
    const auto parentId = parentFolder->id().coreFolderId();
    const auto isRoot = !parentId.isValid();
    auto future = isRoot ? m_coreFs->createFolder(name) : m_coreFs->createFolder(name, parentId, parentFolder->publicKey());
    FutureWorker::run(future, [this, parentFolder, name, isRoot](auto result) {
        if (std::holds_alternative<CoreMessengerStatus>(result)) {
            emit createFolderErrorOccured(tr("Cloud file creation error: %1").arg(name));
            return;
        }
        const auto fsFolder = std::get_if<CloudFsFolder>(&result);
        const auto parentFolderId = fsFolder->info.id;
        // Create folder from core info
        auto createdFolder = createFolderFromInfo(fsFolder->info, parentFolderId);
        if (isRoot) {
            createdFolder->setId(CloudFileId()); // empty id for root
        }
        createdFolder->setLocalPath(QDir(parentFolder->localPath()).filePath(name)); // TODO(fpohtmeh): create local folder?
        emit folderCreated(createdFolder);
    });
}

bool CloudFileSystem::deleteFiles(const CloudFiles &files)
{
    // FIXME(fpohtmeh): implement
    return false;
}

ModifiableCloudFileHandler CloudFileSystem::createFolderFromInfo(const CloudFsFolderInfo &info, const CloudFsFolderId &parentId) const
{
    auto folder = std::make_shared<CloudFile>();
    folder->setId(info.id);
    folder->setParentId(parentId);
    folder->setName(info.name);
    folder->setIsFolder(true);
    folder->setCreatedAt(info.createdAt);
    folder->setUpdatedAt(info.updatedAt);
    folder->setUpdatedBy(info.updatedBy);
    return folder;
}

ModifiableCloudFileHandler CloudFileSystem::createFileFromInfo(const CloudFsFileInfo &info, const CloudFsFolderId &parentId) const
{
    auto file = std::make_shared<CloudFile>();
    file->setId(info.id);
    file->setParentId(parentId);
    file->setName(info.name);
    file->setType(info.type);
    file->setSize(info.size);
    file->setCreatedAt(info.createdAt);
    file->setUpdatedAt(info.updatedAt);
    file->setUpdatedBy(info.updatedBy);
    return file;
}
