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
    m_proxy->setFilterRole(ContactIdRole);
}

ChatsModel::~ChatsModel()
{}

void ChatsModel::setChats(const Chats &chats)
{
    beginResetModel();
    m_chats = chats;
    endResetModel();
    qCDebug(lcModel) << "Chats set";
    emit chatsSet();
}

Chat ChatsModel::createChat(const Contact::Id &contactId)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    Chat chat;
    chat.id = Utils::createUuid();
    chat.timestamp = QDateTime::currentDateTime();
    chat.contactId = contactId;
    m_chats.push_back(chat);
    endInsertRows();
    emit chatCreated(chat);
    return chat;
}

void ChatsModel::resetUnreadCount(const Chat::Id &chatId)
{
    const auto chatRow = findRowById(chatId);
    if (!chatRow) {
        qCWarning(lcModel) << "Chat not found! Id" << chatId;
        return;
    }
    auto &chat = m_chats[*chatRow];
    chat.unreadMessageCount = 0;
    qCDebug(lcModel) << "Unread message count was reset";
    const auto chatIndex = index(*chatRow);
    emit dataChanged(chatIndex, chatIndex, { UnreadMessagesCountRole });
    emit chatUpdated(chat);
}

void ChatsModel::updateLastMessage(const Message &message, const Chat::UnreadCount &unreadMessageCount)
{
    const auto chatRow = findRowById(message.chatId);
    if (!chatRow) {
        qCWarning(lcModel) << "Chat not found! Id" << message.chatId;
        return;
    }

    auto &chat = m_chats[*chatRow];
    if (!chat.lastMessage || chat.lastMessage->id != message.id) {
        qCDebug(lcModel) << "Last message was set to" << message.id;
    }
    chat.lastMessage = message;
    QVector<int> roles{ LastMessageBodyRole, LastEventTimeRole };
    if (unreadMessageCount != chat.unreadMessageCount) {
        chat.unreadMessageCount = unreadMessageCount;
        qCDebug(lcModel) << "Unread message count was set to" << unreadMessageCount;
        roles << UnreadMessagesCountRole;
    }
    const auto chatIndex = index(*chatRow);
    emit dataChanged(chatIndex, chatIndex, roles);
    emit chatUpdated(chat);
}

Optional<Chat> ChatsModel::findById(const Chat::Id &chatId) const
{
    if (const auto row = findRowById(chatId)) {
        return m_chats[*row];
    }
    return NullOptional;
}

Optional<Chat> ChatsModel::findByContact(const Contact::Id &contactId) const
{
    if (const auto row = findRowByContactId(contactId)) {
        return m_chats[*row];
    }
    return NullOptional;
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
    case IdRole:
        return chat.id;
    case ContactIdRole:
        return chat.contactId;
    case LastEventTimeRole:
        return (chat.lastMessage ? qMax(chat.lastMessage->timestamp, chat.timestamp) : chat.timestamp).toString("hh:mm");
    case LastEventTimestampRole:
        return chat.lastMessage ? qMax(chat.lastMessage->timestamp, chat.timestamp) : chat.timestamp;
    case LastMessageBodyRole:
        if (!chat.lastMessage) {
            return QLatin1String("...");
        }
        else if (chat.lastMessage->attachment) {
            return Utils::attachmentDisplayText(*chat.lastMessage->attachment);
        }
        else {
            return Utils::printableMessageBody(*chat.lastMessage);
        }
    case UnreadMessagesCountRole:
        return chat.unreadMessageCount;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChatsModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { ContactIdRole, "contactId" },
        { LastMessageBodyRole, "lastMessageBody" },
        { LastEventTimeRole, "lastEventTime" },
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

Optional<int> ChatsModel::findRowById(const Chat::Id &chatId) const
{
    for (int i = 0, s = m_chats.size(); i < s; ++i) {
        if (m_chats[i].id == chatId) {
            return i;
        }
    }
    return NullOptional;
}

Optional<int> ChatsModel::findRowByLastMessageId(const Message::Id &messageId) const
{
    for (int i = 0, s = m_chats.size(); i < s; ++i) {
        auto m = m_chats[i].lastMessage;
        if (m && m->id == messageId) {
            return i;
        }
    }
    return NullOptional;
}

Optional<int> ChatsModel::findRowByContactId(const Contact::Id &contactId) const
{
    for (int i = 0, s = m_chats.size(); i < s; ++i) {
        if (m_chats[i].contactId == contactId) {
            return i;
        }
    }
    return NullOptional;
}
