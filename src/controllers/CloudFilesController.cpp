//  Copyright (C) 2015-2020 Virgil Security, Inc.
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

#include "CloudFilesController.h"

#include "CloudFilesUpdate.h"
#include "CloudFilesModel.h"
#include "CloudFilesQueue.h"
#include "CloudFilesTable.h"
#include "Controller.h"
#include "FileUtils.h"
#include "Utils.h"

using namespace vm;
using Self = CloudFilesController;

Self::CloudFilesController(const Settings *settings, Models *models, UserDatabase *userDatabase, CloudFileSystem *cloudFileSystem,
                           QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_models(models)
    , m_userDatabase(userDatabase)
    , m_cloudFileSystem(cloudFileSystem)
{
    qRegisterMetaType<CloudFilesUpdate>("CloudFilesUpdate");

    m_rootFolder = std::make_shared<CloudFile>();
    m_rootFolder->setId(CloudFileId::root());
    m_rootFolder->setName(tr("File Manager"));
    m_rootFolder->setIsFolder(true);

    m_hierarchy.push_back(m_rootFolder);

    connect(this, &Self::updateCloudFiles, this, &Self::onUpdateCloudFiles);
    connect(userDatabase, &UserDatabase::opened, this, &Self::setupTableConnections);
    connect(models->cloudFilesQueue(), &CloudFilesQueue::updateCloudFiles, this, &Self::updateCloudFiles);
    connect(cloudFileSystem, &CloudFileSystem::listFetched, this, &Self::onCloudFilesFetched);
    connect(cloudFileSystem, &CloudFileSystem::fetchListErrorOccured, this, &Self::onCloudFilesFetchErrorOccured);
}

CloudFilesModel *Self::model()
{
    return m_models->cloudFiles();
}

void Self::switchToRootFolder()
{
    switchToHierarchy({ m_rootFolder });
}

void Self::setDownloadsDir(const QDir &dir)
{
    m_rootFolder->setLocalPath(dir.absolutePath());
}

void Self::openFile(const QVariant &proxyRow)
{
    const auto cloudFile = model()->file(proxyRow.toInt());
    FileUtils::openUrl(FileUtils::localFileToUrl(cloudFile->localPath()));
}

void Self::switchToFolder(const QVariant &proxyRow)
{
    auto hierarchy = m_hierarchy;
    hierarchy.push_back(model()->file(proxyRow.toInt()));
    switchToHierarchy(hierarchy);
}

void Self::switchToParentFolder()
{
    if (isRoot()) {
        qCWarning(lcController) << "Root cloud folder has no parent";
        return;
    }
    auto hierarchy = m_hierarchy;
    hierarchy.pop_back();
    switchToHierarchy(hierarchy);
}

void Self::refresh()
{
    switchToHierarchy(m_hierarchy);
}

void Self::addFile(const QVariant &attachmentUrl)
{
    const auto url = attachmentUrl.toUrl();
    const auto filePath = FileUtils::urlToLocalFile(url);
    m_models->cloudFilesQueue()->pushUploadFile(filePath, m_hierarchy.back());
}

void Self::deleteFiles()
{
    m_models->cloudFilesQueue()->pushDeleteFiles(m_models->cloudFiles()->selectedFiles());
}

void Self::createFolder(const QString &name)
{
    m_models->cloudFilesQueue()->pushCreateFolder(name, m_hierarchy.back());
}

void Self::setupTableConnections()
{
    qCDebug(lcController) << "Setup database table connections for cloud files...";
    auto table = m_userDatabase->cloudFilesTable();
    connect(table, &CloudFilesTable::errorOccurred, this, &Self::errorOccurred);
    connect(table, &CloudFilesTable::fetched, this, &Self::onDbListFetched);
}

void Self::switchToHierarchy(const FoldersHierarchy &hierarchy)
{
    m_newHierarchy = hierarchy;
    m_userDatabase->cloudFilesTable()->fetch(hierarchy.back());
}

QString CloudFilesController::displayPath() const
{
    QStringList names;
    for (auto &folder : m_hierarchy) {
        names << folder->name();
    }
    return names.join(QLatin1String(" / "));
}

bool CloudFilesController::isRoot() const
{
    return m_hierarchy.size() == 1;
}

void CloudFilesController::setIsLoading(const bool isLoading)
{
    if (isLoading == m_isLoading) {
        return;
    }
    m_isLoading = isLoading;
    emit isLoadingChanged(isLoading);
}

void Self::onDbListFetched(const CloudFileHandler &parentFolder, const ModifiableCloudFiles &cloudFiles)
{
    if (parentFolder->id() != m_newHierarchy.back()->id()) {
        qCDebug(lcController) << "Fetched db folder isn't relevant more" << parentFolder->id();
        return;
    }

    // Update hierarchy
    const auto oldDisplayPath = displayPath();
    const auto oldIsRoot = isRoot();
    m_hierarchy = m_newHierarchy;
    if (displayPath() != oldDisplayPath) {
        emit displayPathChanged(displayPath());
    }
    if (isRoot() != oldIsRoot) {
        emit isRootChanged(isRoot());
    }

    // Emit update
    ListCloudFolderUpdate update;
    update.parentFolder = parentFolder;
    update.files = cloudFiles;
    emit updateCloudFiles(update);

    // Fetch cloud folder
    m_databaseCloudFiles = cloudFiles;
    setIsLoading(true);
    m_cloudFileSystem->fetchList(parentFolder);
}

void CloudFilesController::onCloudFilesFetched(const ModifiableCloudFileHandler &parentFolder, const ModifiableCloudFiles &cloudFiles)
{
    if (parentFolder->id() != m_hierarchy.back()->id()) {
        qCDebug(lcController) << "Fetched cloud folder isn't relevant more" << parentFolder->id();
        return;
    }

    // Emit merge update

    MergeCloudFolderUpdate mergeUpdate;
    mergeUpdate.parentFolder = parentFolder;

    auto oldFiles = m_databaseCloudFiles;
    std::sort(std::begin(oldFiles), std::end(oldFiles), fileIdLess);
    auto newFiles = cloudFiles;
    std::sort(std::begin(newFiles), std::end(newFiles), fileIdLess);
    const auto oldSize = oldFiles.size();
    const auto newSize = newFiles.size();
    size_t oldI = 0;
    size_t newI = 0;
    while (oldI < oldSize && newI < newSize) {
        const auto &oldFile = oldFiles[oldI];
        const auto &newFile = newFiles[newI];
        if (fileIdLess(oldFile, newFile)) {
            mergeUpdate.deleted.push_back(oldFile);
            ++oldI;
        }
        else if (fileIdLess(newFile, oldFile)) {
            mergeUpdate.added.push_back(newFile);
            ++newI;
        }
        else {
            if (!filesAreEqual(oldFile, newFile)) {
                mergeUpdate.updated.push_back(newFile);
            }
            ++oldI;
            ++newI;
        }
    }
    if (oldI < oldSize) {
        mergeUpdate.deleted.insert(mergeUpdate.deleted.end(), oldFiles.begin() + oldI, oldFiles.end());
    }
    if (newI < newSize) {
        mergeUpdate.added.insert(mergeUpdate.added.end(), newFiles.begin() + newI, newFiles.end());
    }

    emit updateCloudFiles(mergeUpdate);

    // Finish loading

    setIsLoading(false);
}

void CloudFilesController::onCloudFilesFetchErrorOccured(const QString &errorText)
{
    Q_UNUSED(errorText)
    setIsLoading(false);
}

void Self::onUpdateCloudFiles(const CloudFilesUpdate &update)
{
    // Update hierarchy
    if (auto upd = std::get_if<MergeCloudFolderUpdate>(&update)) {
        m_hierarchy.back()->update(*upd->parentFolder, CloudFileUpdateSource::ListedParent);
    }
    // Update UI
    m_models->cloudFiles()->updateCloudFiles(update);
    // Update DB
    m_userDatabase->cloudFilesTable()->updateCloudFiles(update);
}

bool CloudFilesController::fileIdLess(const ModifiableCloudFileHandler &a, const ModifiableCloudFileHandler &b)
{
    return a->id() < b->id();
}

bool CloudFilesController::filesAreEqual(const ModifiableCloudFileHandler &a, const ModifiableCloudFileHandler &b)
{
    // Compare common fields
    if (!(a->parentId() == b->parentId() && a->isFolder() == b->isFolder())) {
        return false;
    }
    // Compare fetched fields
    if (!(a->id() == b->id() && a->name() == b->name() && a->createdAt() == b->createdAt() && a->updatedAt() == b->updatedAt())) {
        return false;
    }
    // Compare file fields
    if (!a->isFolder()) {
        return a->type() == b->type() && a->size() == b->size();
    }
    return true;
}
