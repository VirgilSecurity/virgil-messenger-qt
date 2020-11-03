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

#include "database/ChatsTable.h"

#include "Utils.h"
#include "database/core/Database.h"
#include "database/core/DatabaseUtils.h"

using namespace vm;

ChatsTable::ChatsTable(Database *database)
    : DatabaseTable(QLatin1String("chats"), database)
{
    connect(this, &ChatsTable::fetch, this, &ChatsTable::onFetch);
    connect(this, &ChatsTable::createChat, this, &ChatsTable::onCreateChat);
    connect(this, &ChatsTable::resetUnreadCount, this, &ChatsTable::onResetUnreadCount);
    connect(this, &ChatsTable::updateLastMessage, this, &ChatsTable::onUpdateLastMessage);
}

bool ChatsTable::create()
{
    if (DatabaseUtils::readExecQueries(database(), QLatin1String("createChats"))) {
        qCDebug(lcDatabase) << "Chats table was created";
        return true;
    }
    qCCritical(lcDatabase) << "Unable to create chats table";
    return false;
}

void ChatsTable::onFetch()
{
    ScopedConnection connection(*database());
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("selectChats"));
    if (!query) {
        qCCritical(lcDatabase) << "ChatsTable::onFetch error";
        emit errorOccurred(tr("Failed to fetch chats"));
    }
    else {
        auto q = *query;
        Chats chats;
        while (q.next()) {
            Chat chat;
            chat.id = q.value("id").value<Chat::Id>();
            chat.timestamp = q.value("timestamp").toDateTime();
            chat.unreadMessageCount = q.value("unreadMessageCount").value<Chat::UnreadCount>();
            chat.contactId = q.value("contactId").value<Contact::Id>();
            chat.lastMessage = DatabaseUtils::readMessage(q, QLatin1String("lastMessageId"));
            chats.push_back(chat);
        }
        emit fetched(chats);
    }
}

void ChatsTable::onCreateChat(const Chat &chat)
{
    ScopedConnection connection(*database());
    // Check if chat exists
    // TODO(fpohtmeh): remove check and query?
    {
        const DatabaseUtils::BindValues values{{ ":contactId", chat.contactId }};
        auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("checkChatByContact"), values);
        if (!query) {
            qCCritical(lcDatabase) << "ChatsTable::onCreateChat check error";
            emit errorOccurred(tr("Failed to find chat"));
            return;
        }
        if (query->next()) {
            qCInfo(lcDatabase) << "Chat already exists:" << chat.contactId;
            return;
        }
    }
    // Add chat
    const auto lastMessageId = chat.lastMessage ? chat.lastMessage->id : QString();
    const DatabaseUtils::BindValues values {
        { ":id", chat.id },
        { ":timestamp", chat.timestamp },
        { ":contactId", chat.contactId },
        { ":lastMessageId", lastMessageId },
        { ":unreadMessageCount", chat.unreadMessageCount }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertChat"), values);
    if (query) {
        qCDebug(lcDatabase) << "Chat was inserted into table" << chat.contactId;
    }
    else {
        qCCritical(lcDatabase) << "ChatsTable::onCreateChat insertion error";
        emit errorOccurred(tr("Failed to insert chat"));
        return;
    }
}

void ChatsTable::onResetUnreadCount(const Chat &chat)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{{ ":id", chat.id }};
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("resetUnreadCount"), values);
    if (query) {
        qCDebug(lcDatabase) << "Chat unread count was reset" << chat.id;
    }
    else {
        qCCritical(lcDatabase) << "ChatsTable::onResetUnreadCount error";
        emit errorOccurred(tr("Failed to reset unread count"));
    }
}

void ChatsTable::onUpdateLastMessage(const Message &message, const Chat::UnreadCount &unreadMessageCount)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values {
        { ":id", message.chatId },
        { ":lastMessageId", message.id },
        { ":unreadMessageCount", unreadMessageCount }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateLastMessage"), values);
    if (query) {
        qCDebug(lcDatabase) << "Last message was updated for chat" << message.chatId << "unread" << unreadMessageCount;
    }
    else {
        qCCritical(lcDatabase) << "ChatsTable::onUpdateLastMessage error";
        emit errorOccurred(tr("Failed to update last message"));
    }
}
