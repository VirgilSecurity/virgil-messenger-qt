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

#include "FilesProgressModel.h"

#include "Utils.h"
#include "Model.h"

using namespace vm;
using Self = FilesProgressModel;

Self::FilesProgressModel(QObject *parent)
    : ListModel(parent)
{
    qRegisterMetaType<FilesProgressModel *>("FilesProgressModel*");
}

Self::~FilesProgressModel()
{
}

void Self::add(const QString &id, const QString &name, const quint64 bytesTotal, const TransferType transferType)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items.push_back({ id, name, 0, bytesTotal, transferType });
    endInsertRows();
    setActiveCount(true);
}

void Self::setProgress(const QString &id, const quint64 bytesLoaded, const quint64 bytesTotal)
{
    if (const auto index = findById(id); index.isValid()) {
        auto &item = m_items[index.row()];
        if (item.isFailed) {
            return;
        }
        item.bytesLoaded = bytesLoaded;
        item.bytesTotal = bytesTotal;
        emit dataChanged(index, index, { BytesLoadedRole, BytesTotalRole, DisplayProgressRole, IsCompletedRole });
        setActiveCount(isItemCompleted(item) ? calculateActiveCount() : true);
    }
}

void Self::remove(const QString &id)
{
    if (const auto index = findById(id); index.isValid()) {
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        m_items.erase(m_items.begin() + index.row());
        endRemoveRows();
        setActiveCount(calculateActiveCount());
    }
}

void Self::markAsFailed(const QString &id)
{
    if (const auto index = findById(id); index.isValid()) {
        auto &item = m_items[index.row()];
        item.isFailed = true;
        emit dataChanged(index, index, { IsFailedRole, DisplayProgressRole, IsCompletedRole });
        setActiveCount(calculateActiveCount());
    }
}

QModelIndex Self::findById(const QString &id) const
{
    const auto it = std::find_if(std::begin(m_items), std::end(m_items), [&id](auto item) {
        return item.id == id;
    });

    if (it != std::end(m_items)) {
        return index(std::distance(std::begin(m_items), it));
    }

    return QModelIndex();
}

QString Self::displayedProgress(const Item &item)
{
    const auto isDownload = item.transferType == TransferType::Download;
    if (item.isFailed) {
        return isDownload ? tr("Download failed") : tr("Upload failed");
    }
    if (item.bytesLoaded == 0) {
        return isDownload ? tr("Waiting for download...") : tr("Waiting for upload...");
    }
    if (item.bytesLoaded == item.bytesTotal) {
        return isDownload ? tr("Downloaded") : tr("Uploaded");
    }
    return Utils::formattedDataSizeProgress(item.bytesLoaded, item.bytesTotal);
}

bool Self::isItemCompleted(const FilesProgressModel::Item &item)
{
    return item.bytesTotal > 0 && item.bytesLoaded == item.bytesTotal;
}

int Self::calculateActiveCount() const
{
    int activeCount = 0;
    for (const auto &item : m_items) {
        if (item.isFailed || isItemCompleted(item)) {
            continue;
        }
        ++activeCount;
    }
    return activeCount;
}

void Self::setActiveCount(const int activeCount)
{
    if (activeCount != m_activeCount) {
        m_activeCount = activeCount;
        emit activeCountChanged(activeCount);
    }
}

int Self::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.size();
}

QVariant Self::data(const QModelIndex &index, int role) const
{
    const auto &item = m_items[index.row()];
    switch (role) {
        case NameRole:
            return item.name;

        case BytesLoadedRole:
            return item.isFailed ? 0 : item.bytesLoaded;

        case BytesTotalRole:
            return item.bytesTotal;

        case DisplayProgressRole:
            return displayedProgress(item);

        case IsFailedRole:
            return item.isFailed;

        case IsCompletedRole:
            return isItemCompleted(item);

        default:
            return QVariant();
    }
}

QHash<int, QByteArray> Self::roleNames() const
{
    return {
        { NameRole, "name" },
        { BytesLoadedRole, "bytesLoaded" },
        { BytesTotalRole, "bytesTotal" },
        { DisplayProgressRole, "displayProgress" },
        { IsFailedRole, "isFailed" },
        { IsCompletedRole, "isCompleted" }
    };
}
