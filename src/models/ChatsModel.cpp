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

#include "models/ChatsModel.h"

#include <QSortFilterProxyModel>

#include "controllers/Controllers.h"
#include "controllers/ChatsController.h"

using namespace vm;

ChatsModel::ChatsModel(Controllers *controllers, QObject *parent)
    : QAbstractListModel(parent)
    , m_proxy(new QSortFilterProxyModel(this))
{
    m_proxy->setSourceModel(this);
    m_proxy->setSortRole(LastEventTimestampRole);
    m_proxy->sort(0, Qt::DescendingOrder);
    m_proxy->setFilterKeyColumn(0);
    m_proxy->setFilterRole(ContactNameRole);

    connect(controllers->chats(), &ChatsController::chatsFetched, this, &ChatsModel::setChats);
}

ChatsModel::~ChatsModel()
{}

int ChatsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_chats.size();
}

QVariant ChatsModel::data(const QModelIndex &index, int role) const
{
    const auto &chat = m_chats[index.row()];
    switch (role) {
    case ContactNameRole:
        return chat.contact.name;
    case LastEventTimestampRole:
        return chat.lastMessage ? qMax(chat.lastMessage->timestamp, chat.timestamp) : chat.timestamp;
    case LastMessageBodyRole:
        return chat.lastMessage ? chat.lastMessage->body : QString();
    case UnreadMessagesCountRole:
        return chat.unreadMessageCount;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChatsModel::roleNames() const
{
    // NOTE(fpohtmeh): used old names to minimize UI changes
    return {
        { ContactNameRole, "name" },
        { LastMessageBodyRole, "lastMessage" },
        { LastEventTimestampRole, "lastMessageTime" },
        { UnreadMessagesCountRole, "unreadMessageCount" }
    };
}

void ChatsModel::setChats(const Chats &chats)
{
    beginResetModel();
    m_chats = chats;
    endResetModel();
}

void ChatsModel::setFilter(const QString &filter)
{
    if (m_filter == filter) {
        return;
    }
    m_proxy->setFilterFixedString(filter);
    m_filter = filter;
    emit filterChanged(filter);
}
