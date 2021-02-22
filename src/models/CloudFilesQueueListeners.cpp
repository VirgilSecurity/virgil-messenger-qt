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

#include "CloudFilesQueueListeners.h"

#include <QFileInfo>

#include "CloudFilesQueue.h"
#include "FileUtils.h"

using namespace vm;

Q_LOGGING_CATEGORY(lcCloudFilesQueueListener, "cloudfiles-listener");

bool CloudFilesQueueListener::preRunCloudFile(CloudFileOperationSource *source)
{
    Q_UNUSED(source)
    return true;
}

void CloudFilesQueueListener::postRunCloudFile(CloudFileOperationSource *source)
{
    Q_UNUSED(source)
}

void CloudFilesQueueListener::updateCloudFiles(const CloudFilesUpdate &update)
{
    Q_UNUSED(update)
}

bool CloudFilesQueueListener::preRun(OperationSourcePtr source)
{
    const auto fileSource = dynamic_cast<CloudFileOperationSource *>(source.get());
    return preRunCloudFile(fileSource);
}

void CloudFilesQueueListener::postRun(OperationSourcePtr source)
{
    const auto fileSource = dynamic_cast<CloudFileOperationSource *>(source.get());
    postRunCloudFile(fileSource);
}

bool UniqueCloudFileFilter::preRunCloudFile(CloudFileOperationSource *source)
{
    const auto id = createUniqueId(source);
    if (id.isEmpty()) {
        return true;
    }

    QMutexLocker locker(&m_mutex);
    if (!m_ids.contains(id)) {
        m_ids.push_back(id);
        return true;
    }

    qCDebug(lcCloudFilesQueueListener) << "Skipped duplicated operation:" << source->toString();
    notifyNotUnique(source);
    return false;
}

void UniqueCloudFileFilter::postRunCloudFile(CloudFileOperationSource *source)
{
    const auto id = createUniqueId(source);
    if (id.isEmpty()) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_ids.removeAll(id);
}

void UniqueCloudFileFilter::clear()
{
    QMutexLocker locker(&m_mutex);
    m_ids.clear();
}

QString UniqueCloudFileFilter::createUniqueId(CloudFileOperationSource *source) const
{
    switch (source->type()) {
        case SourceType::Download:
            return source->file()->id();
        case SourceType::Upload:
            return source->folder()->id() + QLatin1Char('/') + FileUtils::fileName(source->filePath());
        case SourceType::CreateFolder:
            return source->folder()->id() + QLatin1Char('/') + source->name();
        default:
            return QString();
    }
}

void UniqueCloudFileFilter::notifyNotUnique(CloudFileOperationSource *source)
{
    switch (source->type()) {
        case SourceType::Download:
            emit notificationCreated(tr("File is downloading"), false);
            break;
        case SourceType::Upload:
            emit notificationCreated(tr("File name is not unique"), true);
            break;
        case SourceType::CreateFolder:
            emit notificationCreated(tr("Folder name is not unique"), true);
            break;
        default:
            break;
    }
}

CloudListUpdatingCounter::CloudListUpdatingCounter(QObject *parent)
    : CloudFilesQueueListener(parent)
{
    connect(this, &CloudListUpdatingCounter::increment, this, &CloudListUpdatingCounter::onIncrement);
}

bool CloudListUpdatingCounter::preRunCloudFile(CloudFileOperationSource *source)
{
    if (const auto value = getIncValue(source)) {
        emit increment(value, QPrivateSignal());
    }
    return true;
}

void CloudListUpdatingCounter::postRunCloudFile(CloudFileOperationSource *source)
{
    if (const auto value = getIncValue(source)) {
        emit increment(-value, QPrivateSignal());
    }
}

void CloudListUpdatingCounter::clear()
{
    emit increment(-m_count, QPrivateSignal());
}

int CloudListUpdatingCounter::getIncValue(CloudFileOperationSource *source) const
{
    switch (source->type()) {
    case SourceType::CreateFolder:
    case SourceType::ListFolder:
        return 1;
    case SourceType::Delete:
        return source->files().size();
    default:
        return 0;
    }
}

void CloudListUpdatingCounter::onIncrement(int inc)
{
    m_count += inc;
    emit countChanged(m_count);
}

void CloudFolderUpdateWatcher::subscribe(const CloudFileHandler &cloudFolder)
{
    if (cloudFolder->isRoot()) {
        emit finished(cloudFolder, false);
        return;
    }

    // Subscribe to folder
    qCDebug(lcCloudFilesQueueListener) << "Subscribing to folder" << cloudFolder->name();
    QMutexLocker locker(&m_mutex);
    const auto it = std::find_if(m_items.begin(), m_items.end(), [cloudFolder](auto item) {
        return cloudFolder->id() == item.folder->id();
    });
    if (it == m_items.end()) {
        qCDebug(lcCloudFilesQueueListener) << "Folder is not updated" << cloudFolder->name();
        emit finished(cloudFolder, false);
    }
    else {
        it->subscribed = true;
    }
}

bool CloudFolderUpdateWatcher::preRunCloudFile(CloudFileOperationSource *source)
{
    if (source->type() != SourceType::ListFolder) {
        return true;
    }

    qCDebug(lcCloudFilesQueueListener) << "Starting folder watching:" << source->folder()->name();
    QMutexLocker locker(&m_mutex);
    m_items.push_back({ source->folder() });
    return true;
}

void CloudFolderUpdateWatcher::postRunCloudFile(CloudFileOperationSource *source)
{
    if (source->type() != SourceType::ListFolder) {
        return;
    }

    // Find item
    QMutexLocker locker(&m_mutex);
    const auto folderId = source->folder()->id();
    const auto it = std::find_if(m_items.begin(), m_items.end(), [folderId](auto item) {
        return folderId == item.folder->id();
    });
    if (it == m_items.end()) {
        qCritical(lcCloudFilesQueueListener) << "Interrupted folder watching:" << source->folder()->name();
        throw std::logic_error("Folder must be watched");
    }

    // Emit if subscribed and erase item
    if (it->subscribed) {
        if (!it->updated) {
            qCDebug(lcCloudFilesQueueListener) << "Folder must be updated but it's not. Name:" << it->folder->name();
        }
        emit finished(it->folder, it->updated);
    }
    m_items.erase(it);
    qCDebug(lcCloudFilesQueueListener) << "Finished folder watching:" << source->folder()->name();
}

void CloudFolderUpdateWatcher::updateCloudFiles(const CloudFilesUpdate &update)
{
    auto upd = std::get_if<CloudListCloudFolderUpdate>(&update);
    if (!upd) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    const auto folderId = upd->parentFolder->id();
    const auto it = std::find_if(m_items.begin(), m_items.end(), [folderId](auto item) {
        return folderId == item.folder->id();
    });
    if (it == m_items.end()) {
        return;
    }

    it->folder = upd->parentFolder;
    it->updated = true;
    qCDebug(lcCloudFilesQueueListener) << "Folder is updated:" << upd->parentFolder->name();
}

void CloudFolderUpdateWatcher::clear()
{
    QMutexLocker locker(&m_mutex);
    m_items.clear();
}
