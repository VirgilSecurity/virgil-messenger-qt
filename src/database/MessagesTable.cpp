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

#include "database/MessagesTable.h"

#include "Utils.h"
#include "database/core/Database.h"
#include "database/core/DatabaseUtils.h"

using namespace vm;

MessagesTable::MessagesTable(Database *database)
    : DatabaseTable(QLatin1String("messages"), database)
{
    connect(this, &MessagesTable::setUserId, this, &MessagesTable::onSetUserId);
    connect(this, &MessagesTable::fetchChatMessages, this, &MessagesTable::onFetchChatMessages);
    connect(this, &MessagesTable::fetchNotSentMessages, this, &MessagesTable::onFetchNotSentMessages);
    connect(this, &MessagesTable::createMessage, this, &MessagesTable::onCreateMessage);
    connect(this, &MessagesTable::updateStatus, this, &MessagesTable::onUpdateStatus);
    connect(this, &MessagesTable::markAllAsRead, this, &MessagesTable::onMarkAllAsRead);
}

bool MessagesTable::create()
{
    if (DatabaseUtils::readExecQueries(database(), QLatin1String("createMessages"))) {
        qCDebug(lcDatabase) << "Messages table was created";
        return true;
    }
    qCCritical(lcDatabase) << "Unable to create messages table";
    return false;
}

void MessagesTable::onSetUserId(const UserId &userId)
{
    m_userId = userId;
}

void MessagesTable::onFetchChatMessages(const Chat::Id &chatId)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{{":chatId", chatId }};
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("selectChatMessages"), values);
    if (!query) {
        qCCritical(lcDatabase) << "MessagesTable::onFetch error";
        emit errorOccurred(tr("Failed to fetch messages"));
    }
    else {
        auto q = *query;
        Messages messages;
        while (q.next()) {
            messages.push_back(*DatabaseUtils::readMessage(q));
        }
        emit chatMessagesFetched(messages);
    }
}

void MessagesTable::onFetchNotSentMessages()
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{
        { ":failedStatus", static_cast<int>(Message::Status::Failed) },
        { ":createdStatus", static_cast<int>(Message::Status::Created) },
        { ":userId", m_userId }
    };
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("selectNotSentMessages"), values);
    if (!query) {
        qCCritical(lcDatabase) << "MessagesTable::onFetchFailed error";
        emit errorOccurred(tr("Failed to fetch failed messages"));
    }
    else {
        auto q = *query;
        GlobalMessages messages;
        while (q.next()) {
            const Contact::Id contactId = q.value("contactId").value<Contact::Id>();
            const Contact::Id senderId = q.value("senderId").value<Contact::Id>();
            const Contact::Id recipientId = q.value("recipientId").value<Contact::Id>();
            messages.push_back({ *DatabaseUtils::readMessage(q), m_userId, contactId, senderId, recipientId });
        }
        emit notSentMessagesFetched(messages);
    }
}

void MessagesTable::onCreateMessage(const Message &message)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{
        { ":id", message.id },
        { ":timestamp", message.timestamp },
        { ":chatId", message.chatId },
        { ":authorId", message.authorId },
        { ":status", static_cast<int>(message.status) },
        { ":body", message.body.isEmpty() ? QVariant() : QVariant(message.body) }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertMessage"), values);
    if (query) {
        qCDebug(lcDatabase) << "Message was inserted into table" << message.id;
    }
    else {
        qCCritical(lcDatabase) << "MessagesTable::onCreateMessage error";
        emit errorOccurred(tr("Failed to insert message"));
    }
}

void MessagesTable::onUpdateStatus(const Message::Id &messageId, const Message::Status &status)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values {
        { ":id", messageId },
        { ":status", static_cast<int>(status) }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateMessageStatus"), values);
    if (query) {
        qCDebug(lcDatabase) << "Message status was updated" << messageId << "status" << status;
    }
    else {
        qCCritical(lcDatabase) << "MessagesTable::onUpdateStatus error";
        emit errorOccurred(tr("Failed to update message status"));
    }
}

void MessagesTable::onMarkAllAsRead(const Chat &chat)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{
        { ":chatId", chat.id },
        { ":authorId", chat.contactId },
        { ":readStatus", static_cast<int>(Message::Status::Read) }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("markMessagesAsRead"), values);
    if (query) {
        qCDebug(lcDatabase) << "All messages in chat marked as read" << chat.id << "contact" << chat.contactId;
    }
    else {
        qCCritical(lcDatabase) << "MessagesTable::onMarkAllAsRead error";
        emit errorOccurred(tr("Failed to mark messages as read"));
    }
}
