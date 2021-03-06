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

#include "models/CloudFilesModel.h"

#include "models/ListProxyModel.h"
#include "models/ListSelectionModel.h"
#include "Settings.h"
#include "Utils.h"
#include "Model.h"

using namespace vm;

CloudFilesModel::CloudFilesModel(const Settings *settings, QObject *parent) : ListModel(parent), m_settings(settings)
{
    qRegisterMetaType<CloudFilesModel *>("CloudFilesModel*");

    proxy()->setSortRole(SortRole);
    proxy()->sort(0, Qt::AscendingOrder);
    proxy()->setFilterRole(FilenameRole);

    connect(selection(), &ListSelectionModel::selectedCountChanged, this, &CloudFilesModel::updateDescription);
}

CloudFileHandler CloudFilesModel::file(const int proxyRow) const
{
    return m_files[sourceIndex(proxyRow).row()];
}

ModifiableCloudFileHandler CloudFilesModel::file(const int proxyRow)
{
    return m_files[sourceIndex(proxyRow).row()];
}

CloudFiles CloudFilesModel::selectedFiles() const
{
    CloudFiles files;
    const auto indices = selection()->selectedIndexes();
    for (const auto &i : indices) {
        files.push_back(m_files[i.row()]);
    }
    return files;
}

CloudFileHandler CloudFilesModel::selectedFile() const
{
    const auto indices = selection()->selectedIndexes();
    return indices.empty() ? CloudFileHandler() : m_files[indices.front().row()];
}

void CloudFilesModel::updateCloudFiles(const CloudFilesUpdate &update)
{
    if (auto upd = std::get_if<CachedListCloudFolderUpdate>(&update)) {
        beginResetModel();
        m_files = upd->files;
        endResetModel();
        updateDescription();
    } else if (auto upd = std::get_if<CloudListCloudFolderUpdate>(&update)) {
        for (auto &file : upd->deleted) {
            removeFile(file);
        }
        for (auto &file : upd->updated) {
            updateFile(file, CloudFileUpdateSource::ListedChild);
        }
        for (auto &file : upd->added) {
            addFile(file);
        }
        updateDescription();
    } else if (auto upd = std::get_if<CreateCloudFilesUpdate>(&update)) {
        for (auto &file : upd->files) {
            addFile(file);
        }
        updateDescription();
    } else if (auto upd = std::get_if<DownloadCloudFileUpdate>(&update)) {
        updateDownloadedFile(*upd);
    } else if (auto upd = std::get_if<DeleteCloudFilesUpdate>(&update)) {
        for (auto &file : upd->files) {
            removeFile(file);
        }
        updateDescription();
    } else if (std::holds_alternative<TransferCloudFileUpdate>(update)
               || std::holds_alternative<ListMembersCloudFileUpdate>(update)) {
        return;
    } else {
        throw std::logic_error("Invalid CloudFilesUpdate in CloudFilesModel::updateCloudFiles");
    }
}

int CloudFilesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_files.size();
}

QVariant CloudFilesModel::data(const QModelIndex &index, int role) const
{
    const auto &file = m_files[index.row()];
    switch (role) {
    case FilenameRole:
        return file->name();
    case IsFolderRole:
        return file->isFolder();
    case DisplayFileSizeRole:
        return file->isFolder() ? QString() : Utils::formattedSize(file->size());
    case IsSharedRole:
        return file->isShared();
    case SortRole:
        return QString("%1%2").arg(static_cast<int>(!file->isFolder())).arg(file->name());
    default:
        return ListModel::data(index, role);
    }
}

QHash<int, QByteArray> CloudFilesModel::roleNames() const
{
    return unitedRoleNames(ListModel::roleNames(),
                           { { FilenameRole, "fileName" },
                             { IsFolderRole, "isFolder" },
                             { DisplayFileSizeRole, "displayFileSize" },
                             { IsSharedRole, "isShared" } });
}

QVector<int> CloudFilesModel::rolesFromUpdateSource(const CloudFileUpdateSource source, const bool isFolder)
{
    QVector<int> roles;
    if ((source == CloudFileUpdateSource::ListedParent) || (source == CloudFileUpdateSource::ListedChild)) {
        roles << FilenameRole << SortRole;
    }
    if ((source == CloudFileUpdateSource::ListedChild) && !isFolder) {
        roles << DisplayFileSizeRole;
    }
    return roles;
}

void CloudFilesModel::addFile(const ModifiableCloudFileHandler &file)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_files.push_back(file);
    endInsertRows();
}

void CloudFilesModel::removeFile(const CloudFileHandler &file)
{
    if (const auto index = findById(file->id()); index.isValid()) {
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        m_files.erase(m_files.begin() + index.row());
        endRemoveRows();
    }
}

void CloudFilesModel::updateFile(const CloudFileHandler &file, const CloudFileUpdateSource source)
{
    if (const auto index = findById(file->id()); index.isValid()) {
        m_files[index.row()]->update(*file, source);
        const auto roles = rolesFromUpdateSource(source, file->isFolder());
        if (!roles.isEmpty()) {
            emit dataChanged(index, index, roles);
        }
    }
}

void CloudFilesModel::updateDownloadedFile(const DownloadCloudFileUpdate &update)
{
    const auto &file = update.file;
    if (const auto index = findById(file->id()); index.isValid()) {
        m_files[index.row()]->setFingerprint(update.fingerprint);
        // No roles to update
    }
}

QModelIndex CloudFilesModel::findById(const CloudFileId &cloudFileId) const
{
    const auto it = std::find_if(std::begin(m_files), std::end(m_files),
                                 [&cloudFileId](auto cloudFile) { return cloudFile->id() == cloudFileId; });

    if (it != std::end(m_files)) {
        return index(std::distance(std::begin(m_files), it));
    }

    return QModelIndex();
}

void CloudFilesModel::updateDescription()
{
    // Count folders and files
    int foldersCount = 0;
    int filesCount = 0;
    const bool hasSelection = selection()->hasSelection();
    if (hasSelection) {
        for (auto &file : selectedFiles()) {
            file->isFolder() ? ++foldersCount : ++filesCount;
        }
    } else {
        for (auto &file : m_files) {
            file->isFolder() ? ++foldersCount : ++filesCount;
        }
    }

    // Build new description
    QString newDescription;
    newDescription.append(((foldersCount == 1) ? tr("%1 Folder") : tr("%1 Folders")).arg(foldersCount));
    newDescription.append(QLatin1String(", "));
    newDescription.append(((filesCount == 1) ? tr("%1 File") : tr("%1 Files")).arg(filesCount));
    if (hasSelection) {
        newDescription = tr("Selected: %1").arg(newDescription);
    }

    // Update description
    if (m_description != newDescription) {
        m_description = newDescription;
        emit descriptionChanged(m_description);
    }
}
