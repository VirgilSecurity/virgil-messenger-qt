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

#ifndef VM_DATABASEUTILS_H
#define VM_DATABASEUTILS_H

#include "Message.h"

#include <QSqlQuery>

#include <vector>
#include <utility>
#include <optional>


namespace vm
{
class Database;

class DatabaseUtils
{
public:
    using BindValues = std::vector<std::pair<QString, QVariant>>;

    static bool isValidName(const QString &id);

    static bool readExecQueries(Database *database, const QString &queryId);

    static std::optional<QSqlQuery> readExecQuery(Database *database, const QString &queryId, const BindValues &values = {});

    static ModifiableMessageHandler readMessage(const QSqlQuery &query, const QString &idColumn = {});

private:
    static bool readMessageContentAttachment(const QSqlQuery &query, MessageContentAttachment& attachment);
    static MessageContent readMessageContent(const QSqlQuery &query);
    static MessageContent readMessageContentFile(const QSqlQuery &query);
    static MessageContent readMessageContentPicture(const QSqlQuery &query);
    static MessageContent readMessageContentText(const QSqlQuery &query);
    static MessageContent readMessageContentEncrypted(const QSqlQuery &query);

    static void printQueryRecord(const QSqlQuery &query);
};
}

#endif // VM_DATABASEUTILS_H
