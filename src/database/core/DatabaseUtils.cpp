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
#include "JsonUtils.h"
#include "database/core/Database.h"
#include "AttachmentId.h"
#include "OutgoingMessage.h"
#include "IncomingMessage.h"
#include "MessageContentJsonUtils.h"
#include "MessageContentType.h"

#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

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
        const auto maybeText = FileUtils::readTextFile(queryPath(queryId));
        if (!maybeText) {
            return {};
        }
        QStringList queries;
        const auto texts = maybeText->split(";");
        for (auto &text : texts) {
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
    auto maybeText = FileUtils::readTextFile(queryPath(queryId));
    if (!maybeText) {
        return std::nullopt;
    }

    auto text = std::move(*maybeText);

    for (auto &v : values) {
        if (hasListType(v)) {
            text = replaceListBindValue(text, v);
        }
    }

    auto query = database->createQuery();
    if (!query.prepare(text)) {
        qCCritical(lcDatabase) << "Failed to prepare query:" << query.lastError().databaseText();
        return std::nullopt;
    }

    for (auto &v : values) {
        if (hasListType(v)) {
            // bindValue was processed earlier
        }
        else if (v.first == QLatin1Char('?')) {
            query.addBindValue(v.second);
        }
        else {
            query.bindValue(v.first, v.second);
        }
    }

    if (!query.exec()) {
        qCCritical(lcDatabase) << "Failed to exec query:" << query.lastError().databaseText();
        return std::nullopt;
    }
    return query;
}

bool Self::readMessageContentAttachment(const QSqlQuery &query, MessageContentAttachment& attachment) {
    //
    //  Read generic attachment properties.
    //
    const auto attachmentId = query.value("attachmentId").toString();
    const auto attachmentFingerprint = query.value("attachmentFingerprint").toString();
    const auto attachmentDecryptionKey = query.value("attachmentDecryptionKey").toByteArray();
    const auto attachmentSignature = query.value("attachmentSignature").toByteArray();
    const auto attachmentFilename = query.value("attachmentFilename").toString();
    const auto attachmentLocalPath = query.value("attachmentLocalPath").toString();
    const auto attachmentUrl = query.value("attachmentUrl").toString();
    const auto attachmentSize = query.value("attachmentSize").value<quint64>();
    const auto attachmentEncryptedSize = query.value("attachmentEncryptedSize").value<quint64>();
    const auto attachmentUploadStage = query.value("attachmentUploadStage").toString();
    const auto attachmentDownloadStage = query.value("attachmentDownloadStage").toString();

    attachment.setId(AttachmentId(attachmentId));
    attachment.setFingerprint(attachmentFingerprint);
    attachment.setDecryptionKey(attachmentDecryptionKey);
    attachment.setSignature(attachmentSignature);
    attachment.setFileName(attachmentFilename);
    attachment.setLocalPath(attachmentLocalPath);
    attachment.setRemoteUrl(attachmentUrl);
    attachment.setSize(attachmentSize);
    attachment.setEncryptedSize(attachmentEncryptedSize);
    attachment.setUploadStage(MessageContentUploadStageFromString(attachmentUploadStage));
    attachment.setDownloadStage(MessageContentDownloadStageFromString(attachmentDownloadStage));

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
    const auto attachmentExtras = query.value("attachmentExtras").toString();

    //
    //  Parse extras and write to the content.
    //
    if (MessageContentJsonUtils::readExtras(attachmentExtras, content)) {
        return content;
    }

    return {};
}


MessageContent Self::readMessageContentText(const QSqlQuery &query) {

    const auto messageBody = query.value("messageBody").toString();

    return MessageContentText(messageBody);
}


MessageContent Self::readMessageContentEncrypted(const QSqlQuery &query) {

    const auto messageCiphertext = query.value("messageCiphertext").toByteArray();

    return MessageContentEncrypted(messageCiphertext);
}

void DatabaseUtils::printQueryRecord(const QSqlQuery &query)
{
    const auto r = query.record();
    qCDebug(lcDatabase) << "Printing query record";
    for (int i = 0, s = r.count(); i < s; ++i) {
        qCDebug(lcDatabase).noquote().nospace() << r.field(i).name() << "=>" << r.value(i);
    }
}

MessageContent Self::readMessageContent(const QSqlQuery &query) {

    const auto contentTypeStr = query.value("messageContentType").toString();
    if (contentTypeStr.isEmpty()) {
        return {};
    }

    const auto messageBody = query.value("messageBody").toString();

    const auto contentType = MessageContentTypeFrom(contentTypeStr);
    switch (contentType) {
        case MessageContentType::None:
            return {};

        case MessageContentType::File:
            return Self::readMessageContentFile(query);

        case MessageContentType::Picture:
            return Self::readMessageContentPicture(query);

        case MessageContentType::Encrypted:
            return Self::readMessageContentEncrypted(query);

        case MessageContentType::Text: {
            return MessageContentText(messageBody);
        }

        case MessageContentType::GroupInvitation: {
            return MessageContentJsonUtils::toObject<MessageContentGroupInvitation>(messageBody);
        }

        default:
            return {};
    }
}


ModifiableMessageHandler Self::readMessage(const QSqlQuery &query, const QString &idColumn)
{
    const auto messageIdColumn = idColumn.isEmpty() ? QLatin1String("messageId") : idColumn;
    const auto messageId = query.value(messageIdColumn).toString();
    if (messageId.isEmpty()) {
        return nullptr;
    }

    const auto messageChatId = query.value("messageChatId").toString();
    const auto messageChatType = query.value("messageChatType").toString();
    const auto messageCreatedAt = query.value("messageCreatedAt").toULongLong();
    const auto messageRecipientId = query.value("messageRecipientId").toString();
    const auto messageRecipientUsername = query.value("messageRecipientUsername").toString();
    const auto messageSenderId = query.value("messageSenderId").toString();
    const auto messageSenderUsername = query.value("messageSenderUsername").toString();
    const auto messageIsOutgoing = query.value("messageIsOutgoing").toBool();
    const auto messageStage = query.value("messageStage").toString();

    auto content = readMessageContent(query);
    if (std::holds_alternative<std::monostate>(content)) {
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
    message->setRecipientId(UserId(messageRecipientId));
    message->setSenderId(UserId(messageSenderId));
    message->setCreatedAt(QDateTime::fromTime_t(messageCreatedAt));
    message->setSenderUsername(UserId(messageSenderUsername));
    message->setRecipientUsername(UserId(messageRecipientUsername));
    message->setContent(std::move(content));

    if (ChatTypeFromString(messageChatType) == ChatType::Group) {
        message->setGroupChatInfo(std::make_unique<MessageGroupChatInfo>(GroupId(messageChatId)));
    }

    return message;
}

ModifiableCloudFileHandler Self::readCloudFile(const QSqlQuery &query)
{
    const auto id = query.value("cloudFileId").toString();
    const auto parentId = query.value("cloudFileParentId").toString();
    const auto name = query.value("cloudFileName").toString();
    const auto isFolder = query.value("cloudFileIsFolder").toBool();
    const auto type = query.value("cloudFileType").toString();
    const auto size = query.value("cloudFileSize").value<quint64>();
    const auto createdAt = query.value("cloudFileCreatedAt").toULongLong();
    const auto updatedAt = query.value("cloudFileUpdatedAt").toULongLong();
    const auto updatedBy = query.value("cloudFileUpdatedBy").toString();
    const auto encryptedKey = query.value("cloudFileEncryptedKey").toByteArray();
    const auto publicKey = query.value("cloudFilePublicKey").toByteArray();
    const auto localPath = query.value("cloudFileLocalPath").toString();
    const auto fingerprint = query.value("cloudFileFingerprint").toString();

    auto cloudFile = std::make_shared<CloudFile>();
    if (isFolder) {
        cloudFile->setId(CloudFsFolderId(id));
    }
    else {
        cloudFile->setId(CloudFsFileId(id));
    }
    cloudFile->setParentId(CloudFsFolderId(parentId));
    cloudFile->setName(name);
    cloudFile->setIsFolder(isFolder);
    cloudFile->setType(type);
    cloudFile->setSize(size);
    cloudFile->setCreatedAt(QDateTime::fromTime_t(createdAt));
    cloudFile->setUpdatedAt(QDateTime::fromTime_t(updatedAt));
    cloudFile->setUpdatedBy(UserId(updatedBy));
    cloudFile->setEncryptedKey(encryptedKey);
    cloudFile->setPublicKey(publicKey);
    cloudFile->setLocalPath(localPath);
    cloudFile->setFingerprint(fingerprint);

    return cloudFile;
}

DatabaseUtils::BindValues DatabaseUtils::createNewCloudFileBindings(const CloudFileHandler &cloudFile)
{
    return {
        { ":id", QString(cloudFile->id()) },
        { ":parentId", QString(cloudFile->parentId()) },
        { ":name", cloudFile->name() },
        { ":isFolder", cloudFile->isFolder() },
        { ":type", cloudFile->type() },
        { ":size", cloudFile->size() },
        { ":createdAt", cloudFile->createdAt().toTime_t() },
        { ":updatedAt", cloudFile->updatedAt().toTime_t() },
        { ":updatedBy", QString(cloudFile->updatedBy()) },
        { ":encryptedKey", cloudFile->encryptedKey() },
        { ":publicKey", cloudFile->publicKey() },
        { ":localPath", cloudFile->localPath() },
        { ":fingerprint", cloudFile->fingerprint() }
    };
}

DatabaseUtils::BindValues DatabaseUtils::createUpdatedCloudFileBindings(const CloudFileHandler &cloudFile, const CloudFileUpdateSource source)
{
    if ((source == CloudFileUpdateSource::ListedChild) || (source == CloudFileUpdateSource::ListedParent)) {
        if (cloudFile->isFolder()) {
            return {
                { ":id", QString(cloudFile->id()) },
                { ":parentId", QString(cloudFile->parentId()) },
                { ":name", cloudFile->name() },
                { ":isFolder", cloudFile->isFolder() },
                // No type and size
                { ":createdAt", cloudFile->createdAt().toTime_t() },
                { ":updatedAt", cloudFile->updatedAt().toTime_t() },
                { ":updatedBy", QString(cloudFile->updatedBy()) },
                { ":encryptedKey", cloudFile->encryptedKey() },
                { ":publicKey", cloudFile->publicKey() },
                { ":localPath", cloudFile->localPath() }
                // No fingerprint
            };
        }
        else {
            return {
                { ":id", QString(cloudFile->id()) },
                { ":parentId", QString(cloudFile->parentId()) },
                { ":name", cloudFile->name() },
                { ":isFolder", cloudFile->isFolder() },
                { ":type", cloudFile->type() },
                { ":size", cloudFile->size() },
                { ":createdAt", cloudFile->createdAt().toTime_t() },
                { ":updatedAt", cloudFile->updatedAt().toTime_t() },
                { ":updatedBy", QString(cloudFile->updatedBy()) },
                // No encrypted and public keys
                { ":localPath", cloudFile->localPath() },
                { ":fingerprint", cloudFile->fingerprint() }
            };
        }
    };
    return {};
}

DatabaseUtils::BindValues DatabaseUtils::createDownloadedCloudFileBindings(const CloudFileHandler &cloudFile, const QString &fingerprint)
{
    return {
        { ":id", QString(cloudFile->id()) },
        { ":fingerprint", fingerprint }
    };
}

bool DatabaseUtils::hasListType(const BindValue &bindValue)
{
    switch (bindValue.second.type()) {
    case QVariant::StringList:
    case QVariant::List:
            return true;
        default:
            return false;
    }
}

QString DatabaseUtils::replaceListBindValue(const QString &queryText, const BindValue &bindValue)
{
    QStringList values;
    switch (bindValue.second.type()) {
        case QVariant::StringList:
            for (auto &v : bindValue.second.toStringList()) {
                values << '"' + v + '"';
            }
            break;
        case QVariant::List:
            for (auto &v : bindValue.second.toList()) {
                values << v.toString();
            }
            break;
        default:
            throw std::logic_error("Invalid bindValue, type is not list");
    }

    const auto name = '(' + bindValue.first + ')';
    return QString(queryText).replace(name, '(' + values.join(',') + ')');
}
