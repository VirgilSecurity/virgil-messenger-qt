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
#include "IncomingMessage.h"
#include "OutgoingMessage.h"

using namespace vm;

MessagesTable::MessagesTable(Database *database)
    : DatabaseTable(QLatin1String("messages"), database)
{
    connect(this, &MessagesTable::fetchChatMessages, this, &MessagesTable::onFetchChatMessages);
    connect(this, &MessagesTable::fetchNotSentMessages, this, &MessagesTable::onFetchNotSentMessages);
    connect(this, &MessagesTable::addMessage, this, &MessagesTable::onAddMessage);
    connect(this, &MessagesTable::updateMessage, this, &MessagesTable::onUpdateMessage);
}

bool MessagesTable::create()
{
    if (DatabaseUtils::readExecQueries(database(), QLatin1String("createMessages"))) {
        qCDebug(lcDatabase) << "Messages table was created";
        return true;
    }
    qCCritical(lcDatabase) << "Failed to create messages table";
    return false;
}

void MessagesTable::onFetchChatMessages(const ChatId &chatId)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{{":chatId", QString(chatId) }};
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("selectChatMessages"), values);
    if (!query) {
        qCCritical(lcDatabase) << "MessagesTable::onFetch error";
        emit errorOccurred(tr("Failed to fetch messages"));
    }
    else {
        ModifiableMessages messages;
        while (query->next()) {
            messages.push_back(DatabaseUtils::readMessage(*query));
        }
        emit chatMessagesFetched(messages);
    }
}

void MessagesTable::onFetchNotSentMessages()
{
    ScopedConnection connection(*database());
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("selectNotSentMessages"));
    if (!query) {
        qCCritical(lcDatabase) << "MessagesTable::onFetchFailed error";
        emit errorOccurred(tr("Failed to fetch failed messages"));
    }
    else {
        ModifiableMessages messages;
        while (query->next()) {
            messages.push_back(DatabaseUtils::readMessage(*query));
        }
        emit notSentMessagesFetched(std::move(messages));
    }
}

void MessagesTable::onAddMessage(const MessageHandler &message)
{
    auto messageStage = message->stageString();;

    ScopedConnection connection(*database());
    DatabaseUtils::BindValues values{
        { ":id", QString(message->id()) },
        { ":chatId", QString(message->chatId()) },
        { ":chatType", ChatTypeToString(message->chatType()) },
        { ":createdAt", message->createdAt().toTime_t() },
        { ":authorId", QString(message->senderId()) },
        { ":authorUsername", QString(message->senderUsername()) },
        { ":isOutgoing", message->isOutgoing() },
        { ":stage", messageStage },
        { ":contentType", MessageContentTypeToString(message->content()) }
    };

    if (auto text = std::get_if<MessageContentText>(&message->content())) {
        values.push_back({ ":body", text->text() });
        values.push_back({ ":ciphertext", QVariant() });

    } else if (auto encrypted = std::get_if<MessageContentEncrypted>(&message->content())) {
        values.push_back({ ":ciphertext", encrypted->ciphertext() });
        values.push_back({ ":body", QVariant() });

    } else if (auto attachment = message->contentAsAttachment()) {
        Q_UNUSED(attachment)
        values.push_back({ ":ciphertext", QVariant() });
        values.push_back({ ":body", QVariant() });
    }

    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertMessage"), values);
    if (query) {
        qCDebug(lcDatabase) << "Message was inserted into table" << message->id();
    }
    else {
        qCCritical(lcDatabase) << "MessagesTable::onCreateMessage error";
        emit errorOccurred(tr("Failed to insert message"));
    }
}

void MessagesTable::onUpdateMessage(const MessageUpdate &messageUpdate)
{
    DatabaseUtils::BindValues values;

    if (auto update = std::get_if<IncomingMessageStageUpdate>(&messageUpdate)) {
        values.push_back({ ":id", QString(update->messageId) });
        values.push_back({ ":stage", IncomingMessageStageToString(update->stage) });

    } else if (auto update = std::get_if<OutgoingMessageStageUpdate>(&messageUpdate)) {
        values.push_back({ ":id", QString(update->messageId) });
        values.push_back({ ":stage", OutgoingMessageStageToString(update->stage) });
    }

    if (values.empty()) {
        return;
    }

    ScopedConnection connection(*database());
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateMessageStage"), values);
    if (query) {
        qCDebug(lcDatabase) << "Message stage was updated";
    }
    else {
        qCCritical(lcDatabase) << "MessagesTable::onUpdateMessage error";
        emit errorOccurred(tr("Failed to update message stage"));
    }
}
