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

#include "TransfersModel.h"

#include "FormatUtils.h"
#include "Model.h"

using namespace vm;
using Self = TransfersModel;

Self::TransfersModel(QObject *parent) : ListModel(parent)
{
    qRegisterMetaType<TransfersModel *>("TransfersModel*");
}

Self::~TransfersModel() {}

void Self::add(const QString &id, const QString &name, const quint64 bytesTotal, const TransferType transferType)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_transfers.push_back({ id, name, 0, bytesTotal, transferType });
    endInsertRows();
}

void Self::setProgress(const QString &id, const quint64 bytesLoaded, const quint64 bytesTotal)
{
    if (const auto index = findById(id); index.isValid()) {
        auto &item = m_transfers[index.row()];
        item.bytesLoaded = bytesLoaded;
        item.bytesTotal = bytesTotal;
        emit dataChanged(index, index, { BytesLoadedRole, BytesTotalRole, DisplayProgressRole });
    }
}

void Self::remove(const QString &id)
{
    if (const auto index = findById(id); index.isValid()) {
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        m_transfers.erase(m_transfers.begin() + index.row());
        endRemoveRows();
    }
}

QModelIndex Self::findById(const QString &transferId) const
{
    const auto it = std::find_if(std::begin(m_transfers), std::end(m_transfers),
                                 [&transferId](auto item) { return item.id == transferId; });

    if (it != std::end(m_transfers)) {
        return index(std::distance(std::begin(m_transfers), it));
    }

    return QModelIndex();
}

QString Self::displayedProgress(const Transfer &item)
{
    const auto isDownload = item.transferType == TransferType::Download;
    if (item.bytesLoaded == 0) {
        return isDownload ? tr("Waiting for download...") : tr("Waiting for upload...");
    }
    return FormatUtils::formattedDataSizeProgress(item.bytesLoaded, item.bytesTotal);
}

int Self::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_transfers.size();
}

QVariant Self::data(const QModelIndex &index, int role) const
{
    const auto &item = m_transfers[index.row()];
    switch (role) {
    case IdRole:
        return item.id;

    case NameRole:
        return item.name;

    case BytesLoadedRole:
        return item.bytesLoaded;

    case BytesTotalRole:
        return item.bytesTotal;

    case DisplayProgressRole:
        return displayedProgress(item);

    default:
        return QVariant();
    }
}

QHash<int, QByteArray> Self::roleNames() const
{
    return { { IdRole, "id" },
             { NameRole, "name" },
             { BytesLoadedRole, "bytesLoaded" },
             { BytesTotalRole, "bytesTotal" },
             { DisplayProgressRole, "displayProgress" } };
}
