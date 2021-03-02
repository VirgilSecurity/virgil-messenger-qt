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
#include "ListSelectionModel.h"
#include "Messenger.h"
#include "Models.h"
#include "Settings.h"
#include "UserDatabase.h"
#include "Utils.h"

using namespace vm;
using Self = CloudFilesController;

Self::CloudFilesController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent),
      m_messenger(messenger),
      m_models(models),
      m_userDatabase(userDatabase),
      m_cloudFileSystem(messenger->cloudFileSystem()),
      m_cloudFileObject(new CloudFileObject(messenger, this))
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
    // Selection connections
    connect(models->cloudFiles()->selection(), &ListSelectionModel::changed, this, &Self::onSelectionChanged);
    // Cloud file system connections
    connect(m_cloudFileSystem, &CloudFileSystem::downloadsDirChanged,
            [rootFolder](auto downloadsDir) { rootFolder->setLocalPath(downloadsDir.absolutePath()); });

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

CloudFileHandler Self::currentFile() const
{
    return m_cloudFileObject->cloudFile();
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

void Self::createFolder(const QString &name, const CloudFileMembers &members)
{
    m_models->cloudFilesQueue()->pushCreateFolder(name, m_hierarchy.back(), members);
}

void Self::loadCloudFileMembers()
{
    if (m_cloudFileObject->isShared()) {
        const auto c1 = std::make_shared<Contact>();
        c1->setUserId(UserId("id1"));
        c1->setUsername(QLatin1String("OwnerUser"));
        const auto m1 = std::make_shared<CloudFileMember>(c1, CloudFileMember::Type::Owner);

        const auto c2 = std::make_shared<Contact>();
        c2->setUserId(UserId("id2"));
        c2->setUsername(QLatin1String("Member1User"));
        const auto m2 = std::make_shared<CloudFileMember>(c2, CloudFileMember::Type::Member);

        const auto c3 = std::make_shared<Contact>();
        c3->setUserId(UserId("id3"));
        c3->setUsername(QLatin1String("Member2User"));
        const auto m3 = std::make_shared<CloudFileMember>(c3, CloudFileMember::Type::Member);

        m_cloudFileObject->setMembers({ m1, m2, m3 });
    }
}

void Self::addMembers(const CloudFileMembers &members)
{
    const auto files = m_models->cloudFiles()->selectedFiles();
    m_models->cloudFilesQueue()->pushAddMembers(files, members);
}

void Self::removeSelectedMembers()
{
    const auto files = m_models->cloudFiles()->selectedFiles();
    const auto members = m_cloudFileObject->selectedMembers();
    m_models->cloudFilesQueue()->pushRemoveMembers(files, members);
}

void Self::leaveMembership()
{
    const auto files = m_models->cloudFiles()->selectedFiles();
    const auto member = m_cloudFileObject->findMemberById(m_messenger->currentUser()->id());
    m_models->cloudFilesQueue()->pushRemoveMembers(files, { member });
}

void Self::switchToHierarchy(const FoldersHierarchy &hierarchy)
{
    m_requestedHierarchy = hierarchy;
    m_models->cloudFilesQueue()->pushListFolder(hierarchy.back());
}

QString Self::displayPath() const
{
    QStringList names;
    for (auto &folder : m_hierarchy) {
        names << folder->name();
    }
    return names.join(QLatin1String(" / "));
}

bool Self::isRoot() const
{
    return m_hierarchy.size() == 1;
}

bool Self::isShared() const
{
    return m_hierarchy.back()->isShared();
}

ModifiableCloudFileHandler Self::rootFolder() const
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
            const auto wasRoot = isRoot();
            const auto wasShared = isShared();
            m_hierarchy = m_requestedHierarchy;
            if (displayPath() != oldDisplayPath) {
                emit displayPathChanged(displayPath());
            }
            if (isRoot() != wasRoot) {
                emit isRootChanged(isRoot());
            }
            if (isShared() != wasShared) {
                emit isSharedChanged(isShared());
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

void Self::onSelectionChanged()
{
    const auto file = m_models->cloudFiles()->selectedFile();
    m_cloudFileObject->setCloudFile(file);
}
