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

#include "models/DiscoveredContactsModel.h"

#include <QSortFilterProxyModel>

#include "Settings.h"
#include "Utils.h"

using namespace vm;

DiscoveredContactsModel::DiscoveredContactsModel(QObject *parent)
    : ListModel(parent)
{
    qRegisterMetaType<DiscoveredContactsModel *>("DiscoveredContactsModel*");

    fillDummyContacts();

    proxy()->setSortRole(NameRole);
    proxy()->sort(0, Qt::AscendingOrder);
    proxy()->setFilterRole(NameRole);
}

void DiscoveredContactsModel::fillDummyContacts()
{
    beginResetModel();
    m_list = {
        { QLatin1String("id0"), Contact::Type::Person, QLatin1String("John"), QUrl(), tr("Last seen an hour ago") },
        { QLatin1String("id1"), Contact::Type::Person, QLatin1String("Michael"), QUrl(), tr("Online") },
        { QLatin1String("id2"), Contact::Type::Person, QLatin1String("Donald"), QUrl(), tr("Last seen recently") },
        { QLatin1String("id3"), Contact::Type::Person, QLatin1String("David"), QUrl(), tr("Offline") },
    };
    endResetModel();
}

int DiscoveredContactsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_list.size();
}

QVariant DiscoveredContactsModel::data(const QModelIndex &index, int role) const
{
    const auto &info = m_list[index.row()];
    switch (role) {
    case NameRole:
        return info.name;
    case AvatarUrlRole:
        return info.avatarUrl;
    case LastSeenActivityRole:
        return info.lastSeenActivity;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DiscoveredContactsModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { AvatarUrlRole, "avatarUrl" },
        { LastSeenActivityRole, "lastSeenActivity" },
    };
}
