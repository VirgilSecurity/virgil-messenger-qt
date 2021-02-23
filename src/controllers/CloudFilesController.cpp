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
#include "CloudFilesTransfersModel.h"
#include "CloudFilesQueue.h"
#include "CloudFilesQueueListeners.h"
#include "CloudFilesTable.h"
#include "Controller.h"
#include "DiscoveredContactsModel.h"
#include "FileUtils.h"
#include "Models.h"
#include "Settings.h"
#include "UserDatabase.h"
#include "Utils.h"

using namespace vm;
using Self = CloudFilesController;

Self::CloudFilesController(const Settings *settings, Models *models, UserDatabase *userDatabase,
                           CloudFileSystem *cloudFileSystem, QObject *parent)
    : QObject(parent),
      m_settings(settings),
      m_models(models),
      m_userDatabase(userDatabase),
      m_cloudFileSystem(cloudFileSystem)
{
    qRegisterMetaType<CloudFilesUpdate>("CloudFilesUpdate");

    // Setup root & initial hierarcy
    auto rootFolder = std::make_shared<CloudFile>();
    rootFolder->setId(CloudFileId::root());
    rootFolder->setName(tr("File Manager"));
    rootFolder->setIsFolder(true);
    m_hierarchy.push_back(rootFolder);

    connect(this, &Self::updateCloudFiles, this, &Self::onUpdateCloudFiles);

    // Queue connections
    auto queue = models->cloudFilesQueue();
    connect(queue, &CloudFilesQueue::updateCloudFiles, this, &Self::updateCloudFiles);
    connect(models->cloudFilesTransfers(), &CloudFilesTransfersModel::interruptByCloudFileId, queue,
            &CloudFilesQueue::interruptFileOperation);
    // Cloud file system connections
    connect(cloudFileSystem, &CloudFileSystem::downloadsDirChanged,
            [rootFolder](const QDir &downloadsDir) { rootFolder->setLocalPath(downloadsDir.absolutePath()); });

    // Setup list updating
    auto updatingListener = new CloudListUpdatingCounter(queue);
    queue->addCloudFileListener(updatingListener);
    connect(updatingListener, &CloudListUpdatingCounter::countChanged, this, [this](auto count) {
        if (m_isListUpdating == (count > 0)) {
            return;
        }
        m_isListUpdating = !m_isListUpdating;
        emit isListUpdatingChanged(m_isListUpdating);
    });
}

CloudFilesModel *Self::model()
{
    return m_models->cloudFiles();
}

void Self::switchToRootFolder()
{
    switchToHierarchy({ rootFolder() });
}

void Self::openFile(const QVariant &proxyRow)
{
    const auto cloudFile = model()->file(proxyRow.toInt());
    const auto localPath = cloudFile->localPath();
    bool needDownload = false;
    if (!FileUtils::fileExists(localPath)) {
        needDownload = true;
    } else if (const auto fingerprint = FileUtils::calculateFingerprint(localPath); fingerprint.isEmpty()) {
        qCritical(lcController) << "Failed to calculate fingerprint for file:" << localPath;
        emit notificationCreated(tr("Attachment file is broken"), true);
        return;
    } else if (fingerprint != cloudFile->fingerprint()) {
        qWarning(lcController) << "Fingerprint mismatch, downloading cloud file";
        emit notificationCreated(tr("Downloading cloud file..."), false);
        needDownload = true;
    }

    if (!needDownload) {
        FileUtils::openUrl(FileUtils::localFileToUrl(localPath));
    } else {
        m_models->cloudFilesQueue()->pushDownloadFile(cloudFile, m_hierarchy.back(), [this]() {
            emit notificationCreated(tr("Cloud file was downloaded"), false);
        });
    }
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

void Self::addFiles(const QVariant &fileUrls)
{
    const auto urls = fileUrls.value<QList<QUrl>>();
    for (auto &url : urls) {
        const auto filePath = FileUtils::urlToLocalFile(url);
        m_models->cloudFilesQueue()->pushUploadFile(filePath, m_hierarchy.back());
    }
}

void Self::deleteFiles()
{
    const auto files = m_models->cloudFiles()->selectedFiles();
    m_models->cloudFilesQueue()->pushDeleteFiles(files);
}

void Self::createFolder(const QString &name)
{
    m_models->cloudFilesQueue()->pushCreateFolder(name, m_hierarchy.back());
}

void Self::shareFiles()
{
    const auto files = m_models->cloudFiles()->selectedFiles();
    const auto contacts = m_models->discoveredContacts()->getSelectedContacts();
    m_models->cloudFilesQueue()->pushShareFiles(files, contacts);
}

void Self::switchToHierarchy(const FoldersHierarchy &hierarchy)
{
    m_requestedHierarchy = hierarchy;
    m_models->cloudFilesQueue()->pushListFolder(hierarchy.back());
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

ModifiableCloudFileHandler CloudFilesController::rootFolder() const
{
    return m_hierarchy.front();
}

void Self::onUpdateCloudFiles(const CloudFilesUpdate &update)
{
    bool isRelevantUpdate = true;
    if (auto upd = std::get_if<CachedListCloudFolderUpdate>(&update)) {
        isRelevantUpdate = upd->parentFolder->id() == m_requestedHierarchy.back()->id();

        // Update properties
        if (isRelevantUpdate) {
            const auto oldDisplayPath = displayPath();
            const auto oldIsRoot = isRoot();
            m_hierarchy = m_requestedHierarchy;
            if (displayPath() != oldDisplayPath) {
                emit displayPathChanged(displayPath());
            }
            if (isRoot() != oldIsRoot) {
                emit isRootChanged(isRoot());
            }
        }
    } else if (auto upd = std::get_if<CloudListCloudFolderUpdate>(&update)) {
        isRelevantUpdate = upd->parentFolder->id() == m_requestedHierarchy.back()->id();

        // Update folder in hierarchy
        const auto it = std::find_if(m_hierarchy.begin(), m_hierarchy.end(),
                                     [&upd](auto folder) { return folder->id() == upd->parentFolder->id(); });
        if (it != m_hierarchy.end()) {
            (*it)->update(*upd->parentFolder, CloudFileUpdateSource::ListedParent);
        }
    }

    // Update UI
    if (isRelevantUpdate) {
        m_models->cloudFiles()->updateCloudFiles(update);
    }
    m_models->cloudFilesTransfers()->updateCloudFiles(update);
    // Update DB
    m_userDatabase->cloudFilesTable()->updateCloudFiles(update);
}
