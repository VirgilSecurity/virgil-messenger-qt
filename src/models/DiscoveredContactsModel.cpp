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

#include <QDebug>

using namespace vm;

DiscoveredContactsModel::DiscoveredContactsModel(const Settings *settings, QObject *parent)
    : ListModel(parent)
    , m_settings(settings)
{
    setContacts(); //This is a temp implementation

    proxy()->setSortRole(NameRole);
    proxy()->sort(0, Qt::AscendingOrder);
    proxy()->setFilterRole(NameRole);

    qRegisterMetaType<DiscoveredContactsModel *>();
}

void DiscoveredContactsModel::setContacts()
{
    beginResetModel();
    Contact user1 {"Connor RK800 new", QUrl("https://cdna.artstation.com/p/assets/images/images/011/345/000/large/junghoon-choi-bryandechart.jpg?1529085789") , "Last seen yesterday"};
    Contact user2 {"Connor RK801 new", QUrl("https://cdna.artstation.com/p/assets/images/images/011/345/000/large/junghoon-choi-bryandechart.jpg?1529085789") , "Online"};

    m_list = {user1, user2};
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
    case ImageRole:
        return info.imageSource;
    case StatusRole:
        return info.status;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DiscoveredContactsModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { ImageRole, "imageSource" },
        { StatusRole, "status" },
    };
}
