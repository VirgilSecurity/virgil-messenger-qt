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

#include "CloudFilesTable.h"

#include "core/Database.h"
#include "core/DatabaseUtils.h"

using namespace vm;

CloudFilesTable::CloudFilesTable(Database *database)
    : DatabaseTable(QLatin1String("cloudFiles"), database)
{
    connect(this, &CloudFilesTable::fetch, this, &CloudFilesTable::onFetch);
    connect(this, &CloudFilesTable::updateCloudFile, this, &CloudFilesTable::onUpdateCloudFile);
}

bool CloudFilesTable::create()
{
    if (DatabaseUtils::readExecQueries(database(), QLatin1String("createCloudFiles"))) {
        qCDebug(lcDatabase) << "Cloud files table was created";
        return true;
    }
    qCCritical(lcDatabase) << "Failed to create Cloud files table";
    return false;
}

void CloudFilesTable::createCloudFile(const CloudFileHandler &cloudFile)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues bindValues = {
        { ":id", QString(cloudFile->id()) },
        { ":parentId", QString(cloudFile->parentId()) },
        { ":name", cloudFile->name() },
        { ":isFolder", cloudFile->isFolder() },
        { ":type", cloudFile->type() },
        { ":size", cloudFile->size() },
        { ":createdAt", cloudFile->createdAt().toTime_t() },
        { ":updatedAt", cloudFile->updatedAt().toTime_t() },
        { ":updatedBy", QString(cloudFile->updatedBy()) },
        { ":encryptedKey", cloudFile->encryptedKey() },
        { ":publicKey", cloudFile->publicKey() },
        { ":localPath", cloudFile->localPath() },
        { ":fingerprint", cloudFile->fingerprint() }
    };
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertCloudFile"), bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Cloud file was created" << bindValues.front().second;
    }
    else {
        qCCritical(lcDatabase) << "CloudFilesTable::createCloudFile error";
        emit errorOccurred(tr("Failed to create cloud file"));
    }
}

void CloudFilesTable::deleteCloudFile(const CloudFileId &cloudFileId, bool isFolder)
{
    ScopedConnection connection(*database());
    ScopedTransaction transaction(*database());
    // Delete files/folders recursively
    if (isFolder) {
        const auto parentIds = getCloudFolderFileIds(cloudFileId);
        // TODO(fpohtmeh): process in one query
        for (auto &parentId : parentIds) {
            const DatabaseUtils::BindValues bindValues {{ ":parentId", parentId }};
            auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("deleteCloudFilesByParentId"), bindValues);
            if (!query) {
                qCCritical(lcDatabase) << "CloudFilesTable::deleteCloudFile folder error";
                emit errorOccurred(tr("Failed to delete cloud folder"));
                transaction.rollback();
                return;
            }
        }
    }
    // Delete file
    const DatabaseUtils::BindValues bindValues {{ ":id", QString(cloudFileId) }};
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("deleteCloudFileById"), bindValues);
    if (!query) {
        qCCritical(lcDatabase) << "CloudFilesTable::deleteCloudFile file error";
        emit errorOccurred(tr("Failed to delete cloud file"));
        transaction.rollback();
    }
}

void CloudFilesTable::renameCloudFile()
{
    ScopedConnection connection(*database());
    // TODO(fpohtmeh): implement
}

QStringList CloudFilesTable::getCloudFolderFileIds(const CloudFileId &folderId)
{
    QStringList ids;
    ids << folderId;

    const DatabaseUtils::BindValues values{{ ":folderId", QString(folderId) }};
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("selectCloudFolderFiles"), values);
    if (!query) {
        qCCritical(lcDatabase) << "CloudFilesTable::getCloudFolderFileIds error";
        emit errorOccurred(tr("Failed to get cloud folder file ids"));
        return ids;
    }

    while (query->next()) {
        const bool isFolder = query->value("cloudFileIsFolder").toBool();
        if (!isFolder) {
            continue;
        }
        const CloudFileId id = query->value("cloudFileId").toString();
        ids += getCloudFolderFileIds(id);
    }
    return ids;
}

void CloudFilesTable::onFetch(const CloudFileHandler &folder)
{
    qCDebug(lcDatabase) << "Fetching cloud files...";
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{{ ":folderId", folder->id() }};
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("selectCloudFolderFiles"), values);
    if (!query) {
        qCCritical(lcDatabase) << "CloudFilesTable::onFetch error";
        emit errorOccurred(tr("Failed to fetch cloud files"));
    }
    else {
        ModifiableCloudFiles cloudFiles;
        while (query->next()) {
            cloudFiles.push_back(DatabaseUtils::readCloudFile(*query));
        }
        qCDebug(lcDatabase) << "Fetched cloud files count: " << cloudFiles.size();
        emit fetched(folder, std::move(cloudFiles));
    }
}

void CloudFilesTable::onUpdateCloudFile(const CloudFileUpdate &update)
{
    if (auto upd = std::get_if<CreatedCloudFileUpdate>(&update)) {
        createCloudFile(upd->cloudFile);
    }
    else if (auto upd = std::get_if<DeletedCloudFileUpdate>(&update)) {
        deleteCloudFile(upd->cloudFileId, upd->isFolder);
    }
    else if (auto upd = std::get_if<RenamedCloudFileUpdate>(&update)) {
        renameCloudFile();
    }
}
