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

#include "CloudFileUpdate.h"
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
    qRegisterMetaType<CloudFileUpdate>("CloudFileUpdate");

    m_rootFolder = std::make_shared<CloudFile>();
    m_rootFolder->setName(tr("File Manager"));
    m_rootFolder->setIsFolder(true);

    m_hierarchy.push_back(m_rootFolder);

    connect(userDatabase, &UserDatabase::opened, this, &Self::setupTableConnections);
    connect(models->cloudFilesQueue(), &CloudFilesQueue::updateCloudFile, this, &Self::onUpdateCloudFile);
    connect(cloudFileSystem, &CloudFileSystem::listFetched, this, &Self::onCloudFilesFetched);
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
    connect(table, &CloudFilesTable::fetched, this, &Self::onDbFilesFetched);
}

void Self::switchToHierarchy(const FoldersHierarchy &hierarchy)
{
    m_newHierarchy = hierarchy;
    m_fetchedFromCloud = false;
    m_userDatabase->cloudFilesTable()->fetch(hierarchy.back());
    m_cloudFileSystem->fetchList(hierarchy.back());
}

void CloudFilesController::setCloudFiles(const ModifiableCloudFiles &cloudFiles)
{
    m_models->cloudFiles()->setFiles(cloudFiles);

    const auto oldDisplayPath = displayPath();
    const auto oldIsRoot = isRoot();
    m_hierarchy = m_newHierarchy;
    if (displayPath() != oldDisplayPath) {
        emit displayPathChanged(displayPath());
    }
    if (isRoot() != oldIsRoot) {
        emit isRootChanged(isRoot());
    }
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

void Self::onDbFilesFetched(const CloudFileHandler &folder, const ModifiableCloudFiles &cloudFiles)
{
    if (m_fetchedFromCloud) {
        return; // Cloud list was fetched earlier, skip DB cache
    }
    if (folder->id() != m_newHierarchy.back()->id()) {
        qCWarning(lcController) << "Fetched folder isn't relevant" << folder->id();
        return;
    }
    setCloudFiles(cloudFiles);
}

void CloudFilesController::onCloudFilesFetched(const CloudFileHandler &folder, const ModifiableCloudFiles &cloudFiles)
{
    if (folder->id() != m_newHierarchy.back()->id()) {
        qCWarning(lcController) << "Fetched folder isn't relevant" << folder->id();
        return; // Files were fetched for another folder
    }
    m_fetchedFromCloud = true;
    setCloudFiles(cloudFiles);
    // FIXME(fpohtmeh): replace cloud files in db
}

void Self::onUpdateCloudFile(const CloudFileUpdate &update)
{
    // Update UI
    m_models->cloudFiles()->updateCloudFile(update);
    // Update DB
    m_userDatabase->cloudFilesTable()->updateCloudFile(update);
}
