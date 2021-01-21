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
    connect(this, &CloudFilesTable::updateCloudFiles, this, &CloudFilesTable::onUpdateCloudFiles);
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

bool CloudFilesTable::createFiles(const ModifiableCloudFiles &cloudFiles)
{
    for (auto &f : cloudFiles) {
        if (!createFile(f)) {
            return false;
        }
    }
    return true;
}

bool CloudFilesTable::createFile(const CloudFileHandler &cloudFile)
{
    const auto bindValues = DatabaseUtils::buildCloudFileBindings(cloudFile);
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertCloudFile"), bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Cloud file was created" << bindValues.front().second;
        return true;
    }
    else {
        qCCritical(lcDatabase) << "CloudFilesTable::createFile error";
        emit errorOccurred(tr("Failed to create cloud file"));
        return false;
    }
}

bool CloudFilesTable::updateFiles(const ModifiableCloudFiles &cloudFiles)
{
    for (auto &f : cloudFiles) {
        if (!updateFile(f)) {
            return false;
        }
    }
    return true;
}

bool CloudFilesTable::updateFile(const CloudFileHandler &cloudFile)
{
    const auto bindValues = DatabaseUtils::buildCloudFileBindings(cloudFile);
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateCloudFile"), bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Cloud file was updated" << bindValues.front().second;
        return true;
    }
    else {
        qCCritical(lcDatabase) << "CloudFilesTable::updateFile error";
        emit errorOccurred(tr("Failed to update cloud file"));
        return false;
    }
}

bool CloudFilesTable::deleteFiles(const CloudFiles &cloudFiles)
{
    QStringList deletedIds;
    QStringList deletedParentIds;
    for (auto &file : cloudFiles) {
        const auto &id = file->id();
        deletedIds << id;
        if (file->isFolder()) {
            deletedParentIds << id;
            deletedParentIds += getSubFoldersIds(id);
        }
    }

    if (!deletedIds.empty()) {
        const DatabaseUtils::BindValues bindValues {{ ":ids", deletedIds }};
        const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("deleteCloudFilesByIds"), bindValues);
        if (!query) {
            qCCritical(lcDatabase) << "CloudFilesTable::deleteCloudFile file error";
            emit errorOccurred(tr("Failed to delete cloud file by id"));
            return false;
        }
    }

    if (!deletedParentIds.empty()) {
        const DatabaseUtils::BindValues bindValues {{ ":parentIds", deletedParentIds }};
        const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("deleteCloudFilesByParentIds"), bindValues);
        if (!query) {
            qCCritical(lcDatabase) << "CloudFilesTable::deleteCloudFile file error";
            emit errorOccurred(tr("Failed to delete cloud file by parentId"));
            return false;
        }
    }

    return true;
}

QStringList CloudFilesTable::getSubFoldersIds(const QString &folderId)
{
    const DatabaseUtils::BindValues values{{ ":folderId", QString(folderId) }};
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("selectCloudFolderFiles"), values);
    if (!query) {
        qCCritical(lcDatabase) << "CloudFilesTable::getCloudFolderFileIds error";
        emit errorOccurred(tr("Failed to get cloud folder file ids"));
        return QStringList();
    }

    QStringList ids;
    while (query->next()) {
        const bool isFolder = query->value("cloudFileIsFolder").toBool();
        if (!isFolder) {
            continue;
        }
        const auto id = query->value("cloudFileId").toString();
        ids << id;
        ids += getSubFoldersIds(id);
    }
    return ids;
}

void CloudFilesTable::onFetch(const CloudFileHandler &folder)
{
    qCDebug(lcDatabase) << "Fetching cloud files...";
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{{ ":folderId", QString(folder->id())}};
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

void CloudFilesTable::onUpdateCloudFiles(const CloudFilesUpdate &update)
{
    if (auto upd = std::get_if<ListedCloudFolderUpdate>(&update)) {
        return;
    }

    ScopedConnection connection(*database());
    if (auto upd = std::get_if<MergeCloudFolderUpdate>(&update)) {
        ScopedTransaction transaction(*database());
        if (!deleteFiles(upd->deleted) || !updateFiles(upd->updated) || !createFiles(upd->added)) {
            transaction.rollback();
        }
    }
    else if (auto upd = std::get_if<CreatedCloudFileUpdate>(&update)) {
        createFile(upd->file);
    }
    else if (auto upd = std::get_if<DeletedCloudFilesUpdate>(&update)) {
        ScopedTransaction transaction(*database());
        if (!deleteFiles(upd->files)) {
            transaction.rollback();
        }
    }
    else {
        throw std::logic_error("Invalid CloudFilesUpdate");
    }
}
