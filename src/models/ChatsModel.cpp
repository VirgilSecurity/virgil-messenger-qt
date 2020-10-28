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

#include "Utils.h"

using namespace vm;

ChatsModel::ChatsModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_proxy(new QSortFilterProxyModel(this))
{
    m_proxy->setSourceModel(this);
    m_proxy->setSortRole(LastEventTimestampRole);
    m_proxy->sort(0, Qt::DescendingOrder);
    m_proxy->setFilterKeyColumn(0);
    m_proxy->setFilterRole(ContactNameRole);
}

ChatsModel::~ChatsModel()
{}

void ChatsModel::setChats(const Chats &chats)
{
    beginResetModel();
    m_chats = chats;
    endResetModel();
}

void ChatsModel::addChat(const Contact::Id &contactId)
{
    if (hasChat(contactId)) {
        return;
    }
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    Chat chat;
    chat.id = Utils::createUuid();
    chat.timestamp = QDateTime::currentDateTime();
    // TODO(fpohtmeh): get contact by id from ContactsController
    chat.contact.id = contactId;
    chat.contact.name = contactId;
    m_chats.push_back(chat);
    endInsertRows();
}

void ChatsModel::resetUnreadCount(const Contact::Id &contactId)
{
    const auto chatRow = findChatRow(contactId);
    if (!chatRow) {
        return;
    }
    auto &chat = m_chats[*chatRow];
    chat.unreadMessageCount = 0;
    const auto chatIndex = index(*chatRow);
    emit dataChanged(chatIndex, chatIndex, { UnreadMessagesCountRole });
}

bool ChatsModel::hasChat(const Contact::Id &contactId) const
{
    return bool(findChatRow(contactId));
}

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

void ChatsModel::setFilter(const QString &filter)
{
    if (m_filter == filter) {
        return;
    }
    m_proxy->setFilterFixedString(filter);
    m_filter = filter;
    emit filterChanged(filter);
}

Optional<int> ChatsModel::findChatRow(const Contact::Id &contactId) const
{
    int i = 0;
    for (auto &c : m_chats) {
        if (c.contact.id == contactId) {
            return i;
        }
        ++i;
    }
    return NullOptional;
}
