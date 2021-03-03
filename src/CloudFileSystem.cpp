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
#include "Messenger.h"
#include "Settings.h"
#include "Utils.h"

Q_LOGGING_CATEGORY(lcCloudFileSystem, "cloud-fs")

using namespace vm;

CloudFileSystem::CloudFileSystem(CoreMessenger *coreMessenger, Messenger *messenger)
    : QObject(messenger), m_coreMessenger(coreMessenger), m_messenger(messenger)
{
    qRegisterMetaType<CloudFileRequestId>("CloudFileRequestId");
}

void CloudFileSystem::signIn()
{
    qCDebug(lcCloudFileSystem) << "Sign-in cloud-fs";
    m_coreFs = m_coreMessenger->cloudFs();

    const auto userName = m_messenger->currentUser()->username();
    m_downloadsDir = m_messenger->settings()->cloudFilesDownloadsDir(userName);
    emit downloadsDirChanged(m_downloadsDir);
}

void CloudFileSystem::signOut()
{
    qCDebug(lcCloudFileSystem) << "Sign-out cloud-fs";
    m_coreFs = {};
}

CloudFileRequestId CloudFileSystem::fetchList(const CloudFileHandler &parentFolder)
{
    const auto requestId = ++m_requestId;
    const auto parentFolderId = parentFolder->id().coreFolderId();
    FutureWorker::run(m_coreFs->listFolder(parentFolderId), [this, parentFolder, requestId](auto result) {
        if (std::holds_alternative<CoreMessengerStatus>(result)) {
            emit fetchListErrorOccured(requestId, tr("Cloud folder listing error"));
            return;
        }
        const auto fsFolder = std::get_if<CloudFsFolder>(&result);
        const auto listedFolder = createParentFolderFromInfo(*fsFolder, parentFolder);
        const QDir localDir(listedFolder->localPath());
        // Build list
        ModifiableCloudFiles list;
        for (auto &info : fsFolder->folders) {
            list.push_back(createFolderFromInfo(info, listedFolder->id(), localDir.filePath(info.name)));
        }
        for (auto &info : fsFolder->files) {
            list.push_back(createFileFromInfo(info, listedFolder->id(), localDir.filePath(info.name)));
        }
        emit listFetched(requestId, listedFolder, list);
    });
    return requestId;
}

CloudFileRequestId CloudFileSystem::createFile(const QString &filePath, const CloudFileHandler &parentFolder)
{
    const auto requestId = ++m_requestId;
    const auto parentFolderId = parentFolder->id().coreFolderId();
    const auto tempDir = m_messenger->settings()->cloudFilesCacheDir();
    const auto encFilePath = tempDir.filePath(QLatin1String("upload-") + Utils::createUuid());
    auto future = m_coreFs->createFile(filePath, encFilePath, parentFolderId, parentFolder->publicKey());
    FutureWorker::run(future, [this, filePath, encFilePath, parentFolder, requestId](auto result) {
        if (std::holds_alternative<CoreMessengerStatus>(result)) {
            emit createFileErrorOccurred(requestId, tr("Failed to create file"));
            return;
        }

        const auto fsNewFile = std::get_if<CloudFsNewFile>(&result);
        auto createdFile = createFileFromInfo(fsNewFile->info, parentFolder->id(),
                                              QDir(parentFolder->localPath()).filePath(fsNewFile->info.name));
        emit fileCreated(requestId, createdFile, encFilePath, fsNewFile->uploadLink);
    });
    return requestId;
}

CloudFileRequestId CloudFileSystem::createFolder(const QString &name, const CloudFileHandler &parentFolder,
                                                 const CloudFileMembers &members)
{
    const auto requestId = ++m_requestId;
    const auto parentFolderId = parentFolder->id().coreFolderId();
    auto future = m_coreFs->createFolder(name, members, parentFolderId, parentFolder->publicKey());
    FutureWorker::run(future, [this, parentFolder, name, requestId](auto result) {
        if (std::holds_alternative<CoreMessengerStatus>(result)) {
            emit createFolderErrorOccured(requestId, tr("Failed to create folder"));
            return;
        }

        const auto fsFolder = std::get_if<CloudFsFolder>(&result);
        auto createdFolder = createFolderFromInfo(fsFolder->info, parentFolder->id(),
                                                  QDir(parentFolder->localPath()).filePath(fsFolder->info.name));
        emit folderCreated(requestId, createdFolder);
    });
    return requestId;
}

CloudFileRequestId CloudFileSystem::getDownloadInfo(const CloudFileHandler &file)
{
    const auto requestId = ++m_requestId;
    FutureWorker::run(m_coreFs->getFileDownloadInfo(file->id().coreFileId()), [this, file, requestId](auto result) {
        if (std::holds_alternative<CoreMessengerStatus>(result)) {
            emit getDownloadInfoErrorOccurred(requestId, tr("Get download info error"));
            return;
        }

        const auto fsInfo = std::get_if<CloudFsFileDownloadInfo>(&result);
        emit downloadInfoGot(requestId, file, fsInfo->downloadLink, fsInfo->fileEncryptedKey);
    });
    return requestId;
}

bool CloudFileSystem::decryptFile(const QString &sourcePath, const QByteArray &encryptionKey,
                                  const CloudFileHandler &file, const CloudFileHandler &parentFolder)
{
    const auto status = m_coreFs->decryptFile(sourcePath, file->localPath(), encryptionKey, m_messenger->currentUser(),
                                              createFsFolder(parentFolder));
    return status == CoreMessengerStatus::Success;
}

CloudFileRequestId CloudFileSystem::deleteFiles(const CloudFiles &files)
{
    const auto requestId = ++m_requestId;
    for (auto &file : files) {
        auto future = file->isFolder() ? m_coreFs->deleteFolder(file->id().coreFolderId())
                                       : m_coreFs->deleteFile(file->id().coreFileId());
        FutureWorker::run(future, [this, file, requestId](auto result) {
            if (result == CoreMessengerStatus::Success) {
                emit fileDeleted(requestId, file);
            } else {
                emit deleteFileErrorOccurred(
                        requestId,
                        tr("Cloud file deletion error: %1. Code: %2").arg(file->name()).arg(static_cast<int>(result)));
            }
        });
    }
    return requestId;
}

ModifiableCloudFileHandler CloudFileSystem::createParentFolderFromInfo(const CloudFsFolder &fsFolder,
                                                                       const CloudFileHandler &oldFolder) const
{
    auto folder = createFolderFromInfo(fsFolder.info, oldFolder->parentId(), oldFolder->localPath());
    folder->setId(oldFolder->id()); // avoid root with non-empty id
    folder->update(*oldFolder, CloudFileUpdateSource::ListedParent);
    folder->setEncryptedKey(fsFolder.folderEncryptedKey);
    folder->setPublicKey(fsFolder.folderPublicKey);
    return folder;
}

ModifiableCloudFileHandler CloudFileSystem::createFolderFromInfo(const CloudFsFolderInfo &info,
                                                                 const CloudFileId &parentId,
                                                                 const QString &localPath) const
{
    auto folder = std::make_shared<CloudFile>();
    folder->setId(info.id);
    folder->setParentId(parentId);
    folder->setName(info.name);
    folder->setIsFolder(true);
    folder->setCreatedAt(info.createdAt);
    folder->setUpdatedAt(info.updatedAt);
    folder->setUpdatedBy(info.updatedBy);
    folder->setLocalPath(localPath);
    folder->setSharedGroupId(info.sharedGroupId);
    return folder;
}

ModifiableCloudFileHandler CloudFileSystem::createFileFromInfo(const CloudFsFileInfo &info, const CloudFileId &parentId,
                                                               const QString &localPath) const
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
    file->setLocalPath(localPath);
    return file;
}

CloudFsFolder CloudFileSystem::createFsFolder(const CloudFileHandler &folder) const
{
    CloudFsFolder fsFolder;
    fsFolder.info = createFsFolderInfo(folder);
    fsFolder.folderEncryptedKey = folder->encryptedKey();
    fsFolder.folderPublicKey = folder->publicKey();
    return fsFolder;
}

CloudFsFolderInfo CloudFileSystem::createFsFolderInfo(const CloudFileHandler &folder) const
{
    CloudFsFolderInfo info;
    info.id = folder->id().coreFolderId();
    info.name = folder->name();
    info.createdAt = folder->createdAt();
    info.updatedAt = folder->updatedAt();
    info.updatedBy = folder->updatedBy();
    info.sharedGroupId = folder->sharedGroupId();
    return info;
}
