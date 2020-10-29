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
    connect(this, &MessagesTable::fetch, this, &MessagesTable::processFetch);
    connect(this, &MessagesTable::createMessage, this, &MessagesTable::processCreateMessage);
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

void MessagesTable::processFetch(const Chat::Id &chatId)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values{{":chatId", chatId }};
    auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("selectMessages"), values);
    if (!query) {
        qCCritical(lcDatabase) << "MessagesTable::processFetch error";
        emit errorOccurred(tr("Failed to fetch messages"));
        return;
    }
    auto q = *query;
    Messages messages;
    while (q.next()) {
        messages.push_back(*DatabaseUtils::readMessage(q));
    }
    emit fetched(messages);
}

void MessagesTable::processCreateMessage(const Message &message)
{
    ScopedConnection connection(*database());
    const DatabaseUtils::BindValues values {
        { ":id", message.id },
        { ":timestamp", message.timestamp },
        { ":chatId", message.chatId },
        { ":authorId", message.authorId },
        { ":status", static_cast<int>(message.status) },
        { ":body", message.body }
    };
    const auto query = DatabaseUtils::readExecQuery(database(), QLatin1String("insertMessage"), values);
    if (!query) {
        qCCritical(lcDatabase) << "MessagesTable::processCreateMessage error";
        emit errorOccurred(tr("Failed to insert message"));
        return;
    }
    qCDebug(lcDatabase) << "Message was inserted into table" << message.id;
}
