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
    connect(this, &ChatsTable::fetch, &ChatsTable::processFetch);
    connect(this, &ChatsTable::createChat, &ChatsTable::processCreateChat);
    connect(this, &ChatsTable::resetUnreadCount, &ChatsTable::processResetUnreadCount);
}

bool ChatsTable::create()
{
    if (DatabaseUtils::runQueries(database(), DatabaseUtils::resourcePath("create_chats"))) {
        qCDebug(lcDatabase) << "Chats table was created";
        return true;
    }
    qCCritical(lcDatabase) << "Unable to create chats table";
    return false;
}

#include <QSqlRecord>
void ChatsTable::processFetch()
{
    ScopedConnection connection(*database());
    auto query = DatabaseUtils::readBindQuery(database(), DatabaseUtils::resourcePath("select_chats"));
    if (!query || !query->exec()) {
        qCCritical(lcDatabase) << "ChatsTable::processFetch error" << DatabaseUtils::errorText(query);
        emit fetchErrorOccurred(tr("Failed to fetch chats"));
        return;
    }
    auto q = *query;
    Chats chats;
    while (q.next()) {
        Chat chat;
        chat.id = q.value("id").value<Chat::Id>();
        chat.timestamp = q.value("timestamp").toDateTime();
        chat.unreadMessageCount = q.value("unread_message_count").toUInt();
        // contact
        chat.contact.id = q.value("contact_id").value<Contact::Id>();
        chat.contact.type = q.value("contact_type").value<Contact::Type>();
        chat.contact.name = q.value("contact_name").toString();
        chat.contact.avatarUrl = q.value("contact_avatar_url").toUrl();
        // message
        const auto messageId = q.value("last_message_id").value<Message::Id>();
        if (!messageId.isEmpty()) {
            Message message;
            message.id = messageId;
            message.timestamp = q.value("message_timestamp").toDateTime();
            message.authorId = q.value("message_author_id").toString();
            message.status = q.value("message_status").value<Message::Status>();
            message.body = q.value("message_body").toString();
            chat.lastMessage = message;
        }
        chats.push_back(chat);
    }
    emit fetched(chats);
}

void ChatsTable::processCreateChat(const Contact::Id &contactId)
{
    // FIXME(fpohtmeh): check contactId validness
    ScopedConnection connection(*database());
    // Check if chat exists
    {
        const DatabaseUtils::BindValues values{{ ":contact_id", contactId }};
        auto query = DatabaseUtils::readBindQuery(database(), DatabaseUtils::resourcePath("check_chat_by_contact"), values);
        if (!query || !query->exec()) {
            qCCritical(lcDatabase) << "ChatsTable::processAddChat check error" << DatabaseUtils::errorText(query);
            emit createChatErrorOccurred(tr("Failed to find chat"));
            return;
        }
        if (query->next()) {
            qCInfo(lcDatabase) << "Chat already exists:" << contactId;
            emit chatCreated(contactId);
            return;
        }
    }
    // Add chat
    const DatabaseUtils::BindValues values{
        { ":id", Utils::createUuid() },
        { ":timestamp", DatabaseUtils::currentTimestamp() },
        { ":contact_id", contactId },
        { ":last_message_id", QVariant() },
        { ":unread_message_count", 0 }
    };
    auto query = DatabaseUtils::readBindQuery(database(), DatabaseUtils::resourcePath("insert_chat"), values);
    if (!query || !query->exec()) {
        qCCritical(lcDatabase) << "ChatsTable::processAddChat insertion error" << DatabaseUtils::errorText(query);
        emit createChatErrorOccurred(tr("Failed to insert chat"));
        return;
    }
    qCDebug(lcDatabase) << "Chat was inserted into table" << contactId;
    emit chatCreated(contactId);
}

void ChatsTable::processResetUnreadCount(const Contact::Id &contactId)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{{ ":contact_id", contactId }};
    auto query = DatabaseUtils::readBindQuery(database(), DatabaseUtils::resourcePath("reset_unread_count"), values);
    if (!query || !query->exec()) {
        qCCritical(lcDatabase) << "ChatsTable::processResetUnreadCount error" << DatabaseUtils::errorText(query);
        emit createChatErrorOccurred(tr("Failed to reset unread count"));
    }
    else {
        qCDebug(lcDatabase) << "Chat unread count was reset" << contactId;
        emit unreadCountReset(contactId);
    }
}
