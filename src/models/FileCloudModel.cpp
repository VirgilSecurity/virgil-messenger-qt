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

#include "models/FileCloudModel.h"

#include <QtConcurrent>

#include "models/ListProxyModel.h"
#include "Settings.h"
#include "Utils.h"
#include "Model.h"

using namespace vm;

FileCloudModel::FileCloudModel(const Settings *settings, QObject *parent)
    : ListModel(parent)
    , m_settings(settings)
{
    qRegisterMetaType<FileCloudModel *>("FileCloudModel*");
    qRegisterMetaType<QFileInfoList>("QFileInfoList");

    proxy()->setSortRole(SortRole);
    proxy()->sort(0, Qt::AscendingOrder);
    proxy()->setFilterRole(FilenameRole);

    connect(this, &FileCloudModel::listReady, this, &FileCloudModel::setList);
    connect(&m_updateTimer, &QTimer::timeout, this, &FileCloudModel::invalidateDateTime);
}

void FileCloudModel::setDirectory(const QDir &dir)
{
    if (!m_settings->fileCloudEnabled()) {
        return;
    }
    QtConcurrent::run([=]() {
        auto list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
        emit listReady(list, QPrivateSignal());
    });
}

void FileCloudModel::setEnabled(bool enabled)
{
    if (!m_settings->fileCloudEnabled()) {
        return;
    }
    if (enabled) {
        m_updateTimer.start(m_settings->nowInterval() * 1000);
        m_debugCounter = 1; // debug one update only
    }
    else {
        m_updateTimer.stop();
    }
}

const QFileInfo FileCloudModel::getFileInfo(const int proxyRow) const
{
    return m_list[sourceIndex(proxyRow).row()];
}

int FileCloudModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_list.size();
}

QVariant FileCloudModel::data(const QModelIndex &index, int role) const
{
    const auto &info = m_list[index.row()];
    switch (role) {
    case FilenameRole:
        return info.fileName();
    case IsDirRole:
        return info.isDir();
    case DisplayDateTimeRole: {
        const auto modified = info.fileTime(QFile::FileModificationTime);
        const auto diff = std::chrono::seconds(modified.secsTo(m_now));
        return Utils::formattedElapsedSeconds(diff, m_settings->nowInterval());
    }
    case DisplayFileSize:
        return Utils::formattedSize(info.size());
    case SortRole:
        return QString("%1%2").arg(static_cast<int>(!info.isDir())).arg(info.fileName());
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> FileCloudModel::roleNames() const
{
    return {
        { FilenameRole, "fileName" },
        { IsDirRole, "isDir" },
        { DisplayDateTimeRole, "displayDateTime" },
        { DisplayFileSize, "displayFileSize" },
        // SortRole is hidden
    };
}

void FileCloudModel::setList(const QFileInfoList &list)
{
    beginResetModel();
    m_now = QDateTime::currentDateTime();
    m_list = list;
    endResetModel();
}

void FileCloudModel::invalidateDateTime()
{
    if (m_debugCounter > 0) {
        --m_debugCounter;
        qCDebug(lcModel) << "Updating of filecloud model";
    }
    m_now = QDateTime::currentDateTime();
    emit dataChanged(index(0), index(rowCount() - 1), { DisplayDateTimeRole });
}
