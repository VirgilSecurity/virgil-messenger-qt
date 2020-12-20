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
    connect(this, &ChatsTable::addChat, this, &ChatsTable::onAddChat);
    connect(this, &ChatsTable::resetUnreadCount, this, &ChatsTable::onResetUnreadCount);
    connect(this, &ChatsTable::updateLastMessage, this, &ChatsTable::onUpdateLastMessage);
}

bool ChatsTable::create()
{
    if (DatabaseUtils::readExecQueries(database(), QLatin1String("createChats"))) {
        qCDebug(lcDatabase) << "Chats table was created";
        return true;
    }
    qCCritical(lcDatabase) << "Failed to create chats table";
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
        ModifiableChats chats;
        while (query->next()) {
            auto id = query->value("id").toString();
            auto title = query->value("title").toString();
            auto type = query->value("type").toString();
            auto createdAt = query->value("createdAt").toULongLong();
            auto lastMessage = DatabaseUtils::readMessage(*query, QLatin1String("lastMessageId"));
            auto unreadMessageCount = query->value("unreadMessageCount").value<qsizetype>();

            auto chat = std::make_unique<Chat>();

            chat->setId(ChatId(id));
            chat->setTitle(title);
            chat->setType(Chat::typeFromString(type));
            chat->setCreatedAt(QDateTime::fromTime_t(createdAt));
            chat->setLastMessage(lastMessage);
            chat->setUnreadMessageCount(unreadMessageCount);

            chats.emplace_back(std::move(chat));
        }
        emit fetched(std::move(chats));
    }
}

void ChatsTable::onAddChat(const ChatHandler &chat)
{
    ScopedConnection connection(*database());
    const auto lastMessageId = chat->lastMessage() ? chat->lastMessage()->id() : QString();
    const DatabaseUtils::BindValues values {
        { ":id", QString(chat->id()) },
        { ":type", Chat::typeToString(chat->type()) },
        { ":title", chat->title() },
        { ":createdAt", chat->createdAt().toTime_t() },
        { ":lastMessageId", QVariant() },
        { ":unreadMessageCount", chat->unreadMessageCount() }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertChat"), values);
    if (query) {
        qCDebug(lcDatabase) << "Chat was inserted into table, id: " << chat->id();
    }
    else {
        qCCritical(lcDatabase) << "ChatsTable::onCreateChat insertion error";
        emit errorOccurred(tr("Failed to insert chat"));
        return;
    }
}

void ChatsTable::onResetUnreadCount(const ChatHandler &chat)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{{ ":id", QString(chat->id()) }};
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("resetUnreadCount"), values);
    if (query) {
        qCDebug(lcDatabase) << "Chat unread count was reset, id: " << chat->id();
    }
    else {
        qCCritical(lcDatabase) << "ChatsTable::onResetUnreadCount error";
        emit errorOccurred(tr("Failed to reset unread count"));
    }
}

void ChatsTable::onUpdateLastMessage(const MessageHandler &message, qsizetype unreadMessageCount)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values {
        { ":id", QString(message->chatId()) },
        { ":lastMessageId", QString(message->id()) },
        { ":unreadMessageCount", unreadMessageCount }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateLastMessage"), values);
    if (query) {
        qCDebug(lcDatabase) << "Last message was updated for chat id: " << message->chatId() << ", unread: " << unreadMessageCount;
    }
    else {
        qCCritical(lcDatabase) << "ChatsTable::onUpdateLastMessage error";
        emit errorOccurred(tr("Failed to update last message"));
    }
}
