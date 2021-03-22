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

#include "ListProxyModel.h"
#include "ListSelectionModel.h"
#include "Model.h"
#include "Utils.h"

#include <QSortFilterProxyModel>

using namespace vm;
using Self = ChatsModel;

Self::ChatsModel(QObject *parent) : ListModel(parent)
{
    qRegisterMetaType<ChatsModel *>("ChatsModel*");

    proxy()->setSortRole(LastEventTimestampRole);
    proxy()->sort(0, Qt::DescendingOrder);
    proxy()->setFilterRole(TitleRole);
}

Self::~ChatsModel() { }

void Self::setChats(ModifiableChats chats)
{
    beginResetModel();
    m_chats = std::move(chats);
    endResetModel();
    qCDebug(lcModel) << "Chats set";
}

void Self::clearChats()
{
    beginResetModel();
    m_chats.clear();
    endResetModel();
    qCDebug(lcModel) << "Chats removed";
}

void Self::addChat(ModifiableChatHandler chat)
{
    qCDebug(lcModel) << "New chat added:" << chat->id();
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_chats.push_back(chat);
    endInsertRows();
    emit chatAdded(chat);
}

void Self::deleteChat(const ChatId &chatId)
{
    const auto chatRow = findRowById(chatId);
    if (!chatRow) {
        qCWarning(lcModel) << "Chat not found! Id" << chatId;
        return;
    }
    beginRemoveRows(QModelIndex(), *chatRow, *chatRow);
    m_chats.erase(m_chats.begin() + *chatRow);
    endRemoveRows();
}

void Self::resetUnreadCount(const ChatId &chatId, qsizetype unreadMessageCount)
{
    const auto chatRow = findRowById(chatId);
    if (!chatRow) {
        qCWarning(lcModel) << "Chat not found! Id" << chatId;
        return;
    }
    auto &chat = m_chats[*chatRow];
    if (chat->unreadMessageCount() != unreadMessageCount) {
        qCDebug(lcModel) << "Unread message count was reset to" << unreadMessageCount << "for chat" << chatId;
        chat->setUnreadMessageCount(unreadMessageCount);
        const auto chatIndex = index(*chatRow);
        emit dataChanged(chatIndex, chatIndex, { UnreadMessagesCountRole });
        emit chatUpdated(chat);
    }
}

void Self::updateLastMessage(const MessageHandler &message)
{
    const auto chatRow = findRowById(message->chatId());
    if (!chatRow) {
        qCWarning(lcModel) << "Chat not found! Id" << message->chatId();
        return;
    }

    auto &chat = m_chats[*chatRow];
    if (!chat->lastMessage() || chat->lastMessage()->id() != message->id()) {
        qCDebug(lcModel) << "Last message was set to" << message->id();
    }

    if (!chat->lastMessage() || chat->lastMessage()->createdAt() < message->createdAt()) {
        chat->setLastMessage(message);

        QVector<int> roles { LastMessageBodyRole, LastEventTimeRole };
        const auto chatIndex = index(*chatRow);
        emit dataChanged(chatIndex, chatIndex, roles);
        emit chatUpdated(chat);
    }
}

void ChatsModel::resetLastMessage(const ChatId &chatId)
{
    const auto chatRow = findRowById(chatId);
    if (!chatRow) {
        qCWarning(lcModel) << "Chat not found! Id" << chatId;
        return;
    }

    auto &chat = m_chats[*chatRow];
    if (chat->lastMessage()) {
        qCDebug(lcModel) << "Last message was reset";
    }
    chat->setLastMessage(MessageHandler());
    QVector<int> roles { LastMessageBodyRole, LastEventTimeRole };
    const auto chatIndex = index(*chatRow);
    emit dataChanged(chatIndex, chatIndex, roles);
    emit chatUpdated(chat);
}

void ChatsModel::toggleById(const QString &chatId)
{
    if (const auto row = findRowById(ChatId(chatId))) {
        selection()->toggle(index(*row));
    }
}

void Self::updateGroup(const GroupUpdate &groupUpdate)
{
    const auto nameUpdate = std::get_if<GroupNameUpdate>(&groupUpdate);
    if (!nameUpdate) {
        return;
    }

    const auto chatId = ChatId(GroupUpdateGetId(groupUpdate));
    const auto row = findRowById(chatId);
    if (!row) {
        qCWarning(lcModel) << "Could not find chat to update group:" << chatId;
        return;
    }

    m_chats[*row]->setTitle(nameUpdate->name);
    emit dataChanged(index(*row), index(*row), { TitleRole });
}

ChatHandler Self::findChat(const ChatId &chatId) const
{
    if (const auto row = findRowById(chatId)) {
        return m_chats[*row];
    }
    return nullptr;
}

ModifiableChatHandler Self::findChat(const ChatId &chatId)
{
    if (const auto row = findRowById(chatId)) {
        return m_chats[*row];
    }
    return nullptr;
}

int Self::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_chats.size();
}

QVariant Self::data(const QModelIndex &index, int role) const
{
    const auto &chat = m_chats[index.row()];
    switch (role) {
    case IdRole:
        return QString(chat->id());

    case TitleRole:
        return chat->title();

    case LastEventTimeRole:
        return (chat->lastMessage() ? qMax(chat->lastMessage()->createdAt(), chat->createdAt()) : chat->createdAt())
                .toString("hh:mm");

    case LastEventTimestampRole:
        return chat->lastMessage() ? qMax(chat->lastMessage()->createdAt(), chat->createdAt()) : chat->createdAt();

    case LastMessageBodyRole:
        if (!chat->lastMessage()) {
            return QLatin1String("...");

        } else {
            return Utils::messageContentDisplayText(chat->lastMessage()->content());
        }

    case UnreadMessagesCountRole:
        return chat->unreadMessageCount();

    default:
        return ListModel::data(index, role);
    }
}

QHash<int, QByteArray> Self::roleNames() const
{
    return unitedRoleNames(ListModel::roleNames(),
                           { { IdRole, "id" },
                             { TitleRole, "title" },
                             { LastMessageBodyRole, "lastMessageBody" },
                             { LastEventTimeRole, "lastEventTime" },
                             { UnreadMessagesCountRole, "unreadMessageCount" } });
}

std::optional<int> Self::findRowById(const ChatId &chatId) const
{
    for (int i = 0, s = m_chats.size(); i < s; ++i) {
        if (m_chats[i]->id() == chatId) {
            return i;
        }
    }
    return {};
}
