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
#include "MessageContentJsonUtils.h"

using namespace vm;

MessagesTable::MessagesTable(Database *database)
    : DatabaseTable(QLatin1String("messages"), database)
{
    connect(this, &MessagesTable::fetchChatMessages, this, &MessagesTable::onFetchChatMessages);
    connect(this, &MessagesTable::fetchNotSentMessages, this, &MessagesTable::onFetchNotSentMessages);
    connect(this, &MessagesTable::addMessage, this, &MessagesTable::onAddMessage);
    connect(this, &MessagesTable::deleteChatMessages, this, &MessagesTable::onDeleteChatMessages);
    connect(this, &MessagesTable::deleteGroupInvitationMessage, this, &MessagesTable::onDeleteGroupInvitationMessage);
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
    auto messageStage = message->stageString();

    ScopedConnection connection(*database());
    DatabaseUtils::BindValues values{
        { ":id", QString(message->id()) },
        { ":recipientId", QString(message->recipientId()) },
        { ":senderId", QString(message->senderId()) },
        { ":chatId", QString(message->chatId()) },
        { ":createdAt", message->createdAt().toTime_t() },
        { ":isOutgoing", message->isOutgoing() },
        { ":stage", messageStage },
        { ":contentType", MessageContentTypeToString(message->content()) }
    };

    QString body{};
    QByteArray ciphertext{};

    if (auto text = std::get_if<MessageContentText>(&message->content())) {
        body = text->text();

    } else if (auto encrypted = std::get_if<MessageContentEncrypted>(&message->content())) {
        ciphertext = encrypted->ciphertext();

    } else if (auto invitation = std::get_if<MessageContentGroupInvitation>(&message->content())) {
        body = MessageContentJsonUtils::toString(*invitation);
    }

    values.push_back({ ":body", body });
    values.push_back({ ":ciphertext", ciphertext });

    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertMessage"), values);
    if (query) {
        qCDebug(lcDatabase) << "Message was inserted into table" << message->id();
    }
    else {
        qCCritical(lcDatabase) << "MessagesTable::onCreateMessage error";
        emit errorOccurred(tr("Failed to insert message"));
    }
}

void MessagesTable::onDeleteChatMessages(const ChatId &chatId)
{
    const DatabaseUtils::BindValues values {{ ":id", QString(chatId) }};
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("deleteMessagesByChatId"), values);
    if (query) {
        qCDebug(lcDatabase) << "Chat messages was removed, chatId:" << chatId;
    }
    else {
        qCCritical(lcDatabase) << "MessagesTable::onDeleteChatMessages deletion error";
        emit errorOccurred(tr("Failed to delete chat messages"));
    }
}

void MessagesTable::onDeleteGroupInvitationMessage(const ChatId &chatId)
{
    const DatabaseUtils::BindValues values {
        { ":id", QString(chatId) },
        { ":contentType", MessageContentTypeToString(MessageContentType::GroupInvitation) }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("deleteGroupInvitationMessageByChatId"), values);
    if (query) {
        qCDebug(lcDatabase) << "Group invitation message was removed, chatId:" << chatId;
    }
    else {
        qCCritical(lcDatabase) << "MessagesTable::onDeleteGroupInvitationMessage deletion error";
        emit errorOccurred(tr("Failed to delete group invitation message"));
    }
}

void MessagesTable::onUpdateMessage(const MessageUpdate &messageUpdate)
{
    DatabaseUtils::BindValues bindValues;

    if (auto update = std::get_if<IncomingMessageStageUpdate>(&messageUpdate)) {
        bindValues.push_back({ ":id", QString(update->messageId) });
        bindValues.push_back({ ":stage", IncomingMessageStageToString(update->stage) });

    } else if (auto update = std::get_if<OutgoingMessageStageUpdate>(&messageUpdate)) {
        bindValues.push_back({ ":id", QString(update->messageId) });
        bindValues.push_back({ ":stage", OutgoingMessageStageToString(update->stage) });
    }

    if (bindValues.empty()) {
        return;
    }

    ScopedConnection connection(*database());
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("updateMessageStage"), bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Message was updated" << bindValues.front().second << bindValues.back();
    }
    else {
        qCCritical(lcDatabase) << "MessagesTable::onUpdateMessage error";
        emit errorOccurred(tr("Failed to update message stage"));
    }
}
