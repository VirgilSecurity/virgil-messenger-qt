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

#include "database/core/DatabaseUtils.h"

#include "FileUtils.h"
#include "database/core/Database.h"
#include "AttachmentId.h"
#include "OutgoingMessage.h"
#include "IncomingMessage.h"

#include <QSqlError>
#include <QSqlQuery>

using namespace vm;
using Self = DatabaseUtils;

namespace
{
    QString queryPath(const QString &fileName)
    {
        return QString(":/resources/database/%1.sql").arg(fileName);
    }

    std::optional<QStringList> readQueryTexts(const QString &queryId)
    {
        const auto text = FileUtils::readTextFile(queryPath(queryId));
        QStringList queries;
        const auto texts = text.split(";");
        for (auto text : texts) {
            auto query = text.trimmed();
            if (!query.isEmpty()) {
                queries << query;
            }
        }
        return queries;
    }
}

bool Self::isValidName(const QString &id)
{
    // TODO(fpohtmeh): add regexp check
    return !id.isEmpty();
}

bool Self::readExecQueries(Database *database, const QString &queryId)
{
    const auto texts = readQueryTexts(queryId);
    if (!texts) {
        return false;
    }
    for (auto text : *texts) {
        auto query = database->createQuery();
        if (!query.exec(text)) {
            qCCritical(lcDatabase) << "Failed to run query:" << text;
            return false;
        }
    }
    return true;
}

std::optional<QSqlQuery> Self::readExecQuery(Database *database, const QString &queryId, const BindValues &values)
{
    const auto text = FileUtils::readTextFile(queryPath(queryId));
    if (text.isEmpty()) {
        return {};
    }
    auto query = database->createQuery();
    if (!query.prepare(text)) {
        qCCritical(lcDatabase) << "Failed to prepare query:" << query.lastError().databaseText();
        return {};
    }
    for (auto &v : values) {
        query.bindValue(v.first, v.second);
    }
    if (!query.exec()) {
        qCCritical(lcDatabase) << "Failed to exec query:" << query.lastError().databaseText();
        return {};
    }
    return query;
}

bool Self::readMessageContentAttachment(const QSqlQuery &query, MessageContentAttachment& attachment) {
    //
    //  Read generic attachment properties.
    //
    auto attachmentId = query.value("attachmentId").toString();
    auto attachmentFingerprint = query.value("attachmentFingerprint").toString();
    auto attachmentFilename = query.value("attachmentFilename").toString();
    auto attachmentLocalPath = query.value("attachmentLocalPath").toString();
    auto attachmentUrl = query.value("attachmentUrl").toString();
    auto attachmentSize = query.value("attachmentSize").value<quint64>();
    auto attachmentEncryptedSize = query.value("attachmentEncryptedSize").value<quint64>();
    auto attachmentUploadStage = query.value("attachmentUploadStage").toString();
    auto attachmentDownloadStage = query.value("attachmentDownloadStage").toString();

    attachment.setId(AttachmentId(attachmentId));
    attachment.setFingerprint(attachmentFingerprint);
    attachment.setFilename(attachmentFilename);
    attachment.setLocalPath(attachmentLocalPath);
    attachment.setRemoteUrl(attachmentUrl);
    attachment.setSize(attachmentSize);
    attachment.setEncryptedSize(attachmentEncryptedSize);
    attachment.setUploadStage(MessageContentAttachment::uploadStageFromString(attachmentUploadStage));
    attachment.setDownloadStage(MessageContentAttachment::downloadStageFromString(attachmentDownloadStage));

    return true;
}


MessageContent Self::readMessageContentFile(const QSqlQuery &query) {

    MessageContentFile content;

    if (Self::readMessageContentAttachment(query, content)) {
        return content;
    }

    return {};
}


MessageContent Self::readMessageContentPicture(const QSqlQuery &query) {

    MessageContentPicture content;

    if (!Self::readMessageContentAttachment(query, content)) {
        return {};
    }

    //
    //  Read extras.
    //
    auto attachmentExtras = query.value("attachmentExtras").toString();

    //
    //  Parse extras and write to the content.
    //
    if (!content.parseExtra(attachmentExtras)) {
        return {};
    }

    return content;
}


MessageContent Self::readMessageContentText(const QSqlQuery &query) {

    auto messageBody = query.value("messageBody").toString();

    return MessageContentText(messageBody);
}


MessageContent Self::readMessageContentEncrypted(const QSqlQuery &query) {

    auto messageCiphertext = query.value("messageCiphertext").toString();

    return MessageContentEncrypted(messageCiphertext);
}


MessageContent Self::readMessageContent(const QSqlQuery &query) {

    auto contentType = query.value("contentType").toString();
    if (contentType.isEmpty()) {
        return nullptr;
    }

    switch (MessageContent::typeFromString(contentType)) {
        case MessageContent::Type::File:
            return Self::readMessageContentFile(query);

        case MessageContent::Type::Picture:
            return Self::readMessageContentPicture(query);

        case MessageContent::Type::Encrypted:
            return Self::readMessageContentEncrypted(query);

        case MessageContent::Type::Text:
            return Self::readMessageContentText(query);
    }
}


ModifiableMessageHandler Self::readMessage(const QSqlQuery &query, const QString &idColumn)
{
    const auto messageIdColumn = idColumn.isEmpty() ? QLatin1String("messageId") : idColumn;
    const auto messageId = query.value(messageIdColumn).toString();
    if (messageId.isEmpty()) {
        return nullptr;
    }

    auto messageChatId = query.value("messageChatId").toString();
    auto messageChatType = query.value("chatType").toString();
    auto messageCreatedAt = query.value("messageCreatedAt").toULongLong();
    auto messageAuthorId = query.value("messageAuthorId").toString();
    auto messageIsOutgoing = query.value("messageIsOutgoing").toBool();
    auto messageStage = query.value("messageStage").toString();

    auto content = readMessageContent(query);
    if (!content) {
        qCCritical(lcDatabase) << "Read message without content with id: " << messageId;
        return nullptr;
    }

    ModifiableMessageHandler message;
    if (messageIsOutgoing) {
        auto outgoingMessage = std::make_unique<OutgoingMessage>();
        outgoingMessage->setStage(OutgoingMessage::stageFromString(messageStage));
        message = std::move(outgoingMessage);

    } else {
        auto incomingMessage = std::make_unique<IncomingMessage>();
        incomingMessage->setStage(IncomingMessage::stageFromString(messageStage));
        message = std::move(incomingMessage);
    }

    message->setId(MessageId(messageId));
    message->setChatId(ChatId(messageChatId));
    message->setChatType(ChatTypeFromString(messageChatType));
    message->setCreatedAt(QDateTime::fromTime_t(messageCreatedAt));
    message->setSenderId(UserId(messageAuthorId));
    message->setContent(std::move(content));

    return message;
}
