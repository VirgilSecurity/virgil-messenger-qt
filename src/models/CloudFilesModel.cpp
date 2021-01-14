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

#include <QtConcurrent>

#include "models/ListProxyModel.h"
#include "models/ListSelectionModel.h"
#include "Settings.h"
#include "Utils.h"
#include "Model.h"

using namespace vm;

CloudFilesModel::CloudFilesModel(const Settings *settings, QObject *parent)
    : ListModel(parent)
    , m_settings(settings)
{
    qRegisterMetaType<CloudFilesModel *>("CloudFilesModel*");
    qRegisterMetaType<QFileInfoList>("QFileInfoList");

    proxy()->setSortRole(SortRole);
    proxy()->sort(0, Qt::AscendingOrder);
    proxy()->setFilterRole(FilenameRole);

    selection()->setMultiSelect(true);

    connect(this, &CloudFilesModel::listReady, this, &CloudFilesModel::setList);
    connect(&m_updateTimer, &QTimer::timeout, this, &CloudFilesModel::invalidateDateTime);
}

void CloudFilesModel::setDirectory(const CloudFileHandler &cloudDir)
{
    QtConcurrent::run([this, cloudDir]() {
        const QDir dir(cloudDir->localPath());
        const auto list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
        emit listReady(list, QPrivateSignal());
    });
}

void CloudFilesModel::setEnabled(bool enabled)
{
    if (enabled) {
        m_updateTimer.start(m_settings->nowInterval() * 1000);
        m_debugCounter = 1; // debug one update only
    }
    else {
        m_updateTimer.stop();
    }
}

CloudFileHandler CloudFilesModel::getFile(const int proxyRow) const
{
    return m_list[sourceIndex(proxyRow).row()];
}

CloudFiles CloudFilesModel::getSelectedFiles() const
{
    CloudFiles files;
    const auto indices = selection()->selectedIndexes();
    for (const auto &i : indices) {
        files.push_back(m_list[i.row()]);
    }
    return files;
}

int CloudFilesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_list.size();
}

QVariant CloudFilesModel::data(const QModelIndex &index, int role) const
{
    const auto &file = m_list[index.row()];
    switch (role) {
    case FilenameRole:
        return file->name();
    case IsDirRole:
        return file->isDirectory();
    case DisplayDateTimeRole: {
        const auto diff = std::chrono::seconds(file->createdAt().secsTo(m_now));
        return Utils::formattedElapsedSeconds(diff, m_settings->nowInterval());
    }
    case DisplayFileSize:
        return Utils::formattedSize(file->size());
    case SortRole:
        return QString("%1%2").arg(static_cast<int>(!file->isDirectory())).arg(file->name());
    default:
        return ListModel::data(index, role);
    }
}

QHash<int, QByteArray> CloudFilesModel::roleNames() const
{
    return unitedRoleNames(ListModel::roleNames(), {
        { FilenameRole, "fileName" },
        { IsDirRole, "isDir" },
        { DisplayDateTimeRole, "displayDateTime" },
        { DisplayFileSize, "displayFileSize" }
    });
}

void CloudFilesModel::setList(const QFileInfoList &list)
{
    beginResetModel();
    m_now = QDateTime::currentDateTime();
    const auto s = list.size();
    m_list.resize(s);
    for (int i = 0; i < s; ++i) {
        m_list[i] = std::make_shared<CloudFile>(list[i]);
    }
    endResetModel();
}

void CloudFilesModel::invalidateDateTime()
{
    if (m_debugCounter > 0) {
        --m_debugCounter;
        qCDebug(lcModel) << "Updating of cloud files model";
    }
    m_now = QDateTime::currentDateTime();
    emit dataChanged(index(0), index(rowCount() - 1), { DisplayDateTimeRole });
}
