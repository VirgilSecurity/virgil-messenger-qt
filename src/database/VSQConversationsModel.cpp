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

#include "models/ConversationsModel.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

#include "Utils.h"
#include "models/AttachmentsModel.h"
#include "models/ChatsModel.h"

ConversationsModel::ConversationsModel(ChatsModel *chats, AttachmentsModel *attachments, QObject *parent)
    : QSqlQueryModel(parent)
    , m_chats(chats)
    , m_attachments(attachments)
{}

QVariant ConversationsModel::data(const QModelIndex &index, int role) const
{
    if (role < Qt::UserRole) {
        return QSqlQueryModel::data(index, role);
    }

    const QSqlRecord currRecord = record(index.row());
    const int column = role - Qt::UserRole;
    if (column == FirstMessageInARowColumn) {
        const QSqlRecord prevRecord = record(index.row() - 1);
        const QVariant prevMsgDir = prevRecord.value(AuthorColumn);
        const QVariant currMsgDir = currRecord.value(AuthorColumn);
        const QDateTime prevTimestamp = prevRecord.value(TimestampColumn).toDateTime();
        const QDateTime currTimestamp = currRecord.value(TimestampColumn).toDateTime();

        // Check if previous message is from the same author
        const bool sameAuthor = currMsgDir != prevMsgDir;

        // Check if the message was sent in last 5 min
        const bool isInFiveMinRange = prevTimestamp.addSecs(5 * 60) > currTimestamp;

        // Message is considered to be the first in a row when it
        // sends in a range of 1 min with previous message and
        // from the same author
        return sameAuthor || !isInFiveMinRange;
    }
    else if (column == MessageInARowColumn) {
        const QSqlRecord nextRecord = record(index.row() + 1);
        const QVariant nextMsgDir = nextRecord.value(AuthorColumn);
        const QVariant currMsgDir = currRecord.value(AuthorColumn);
        return currMsgDir == nextMsgDir;
    }
    else if (column == DayColumn) {
        return currRecord.value(TimestampColumn).toDate();
    }
    else if (column == AttachmentIdColumn || column == AttachmentLocalUrlColumn || column == AttachmentLocalPreviewColumn) {
        return currRecord.value(column).toString();
    }
    else if (column == AttachmentSizeColumn) {
        return Utils::formattedFileSize(currRecord.value(AttachmentSizeColumn).toInt());
    }
    else if (column == AttachmentTypeColumn) {
        const auto type = static_cast<Enums::AttachmentType>(currRecord.value(AttachmentTypeColumn).toInt());
        return QVariant::fromValue(type);
    }
    else {
        return currRecord.value(column);
    }
}

void ConversationsModel::createTable()
{
    const QString queryString = QString(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "  id TEXT NOT NULL UNIQUE,"
        "  author INTEGER NOT NULL,"
        "  chat_id TEXT NOT NULL,"
        "  timestamp TEXT NOT NULL,"
        "  message TEXT NOT NULL,"
        "  status INTEGER NOT NULL,"
        "  FOREIGN KEY(chat_id) REFERENCES %2 (id)"
        ")"
    ).arg(m_tableName).arg(m_chats->tableName());
    QSqlQuery query(queryString);
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to create converstations table:") << query.lastError().text();
        return;
    }
    const QString indexQueryString = QString(
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_%1_id ON %1 (id);"
    ).arg(m_tableName);
    QSqlQuery indexQuery(indexQueryString);
    if (!indexQuery.exec()) {
        qCritical() << QLatin1String("Failed to create converstations indices:") << query.lastError().text();
    }
}

void ConversationsModel::resetModel(const QString &chatId)
{
    m_chatId = chatId;
    setQuery(buildQuery(chatId, QLatin1String()));
}

QSqlQuery ConversationsModel::buildQuery(const QString &chatId, const QString &condition) const
{
    // body
    const QString body = QString(
        "SELECT "
        "  C.id as id,"
        "  C.author as author,"
        "  C.chat_id as chat_id,"
        "  C.timestamp as timestamp,"
        "  C.message as message,"
        "  C.status as status,"
        "  IFNULL(A.id, '') as attachment_id,"
        "  IFNULL(A.size, 0) as attachment_size,"
        "  IFNULL(A.type, 0) as attachment_type,"
        "  IFNULL(A.local_url, '') as attachment_local_url,"
        "  IFNULL(A.local_preview, '') as attachment_local_preview "
        "FROM %1 C "
        "LEFT JOIN %2 A ON C.id = A.message_id "
    ).arg(m_tableName).arg(m_attachments->tableName());
    // condition
    QStringList conditionList;
    if (!chatId.isEmpty())
        conditionList << QString("chat_id = '%1'").arg(chatId);
    if (!condition.isEmpty())
        conditionList << condition;
    const QString conditionString = conditionList.join(QLatin1String(" AND "));
    // sorting
    const QLatin1String sorting(" ORDER BY timestamp");

    QString queryString(body);
    if (!conditionString.isEmpty())
        queryString.append(QLatin1String(" WHERE ") + conditionString);
    queryString.append(sorting);
    return QSqlQuery(queryString);
}

QHash<int, QByteArray> ConversationsModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::UserRole + IdColumn] = "id";
    names[Qt::UserRole + AuthorColumn] = "author";
    names[Qt::UserRole + ChatIdColumn] = "chat_id";
    names[Qt::UserRole + TimestampColumn] = "timestamp";
    names[Qt::UserRole + MessageColumn] = "message";
    names[Qt::UserRole + StatusColumn] = "status";
    names[Qt::UserRole + AttachmentIdColumn] = "attachmentId";
    names[Qt::UserRole + AttachmentSizeColumn] = "attachmentSize";
    names[Qt::UserRole + AttachmentTypeColumn] = "attachmentType";
    names[Qt::UserRole + AttachmentLocalUrlColumn] = "attachmentLocalUrl";
    names[Qt::UserRole + AttachmentLocalPreviewColumn] = "attachmentLocalPreview";
    names[Qt::UserRole + FirstMessageInARowColumn] = "firstMessageInARow";
    names[Qt::UserRole + MessageInARowColumn] = "messageInARow";
    names[Qt::UserRole + DayColumn] = "day";
    return names;
}

void ConversationsModel::writeMessage(const QString &messageId, const QString &message, const OptionalAttachment &attachment,
                                         const QString &recipientId, const StMessage::Author author, const EnMessageStatus status)
{
    const auto chatId = m_chats->createPrivateChat(recipientId);

    // Write message
    const QString queryString = QString(
        "INSERT INTO %1 (id, author, chat_id, timestamp, message, status)"
        "VALUES (:id, :author, :chat_id, :timestamp, :message, :status)"
    ).arg(m_tableName);
    QSqlQuery query;
    query.prepare(queryString);
    query.bindValue(QLatin1String(":id"), messageId);
    query.bindValue(QLatin1String(":author"), static_cast<int>(author));
    query.bindValue(QLatin1String(":chat_id"), *chatId);
    query.bindValue(QLatin1String(":timestamp"), Utils::currentIsoDateTime());
    query.bindValue(QLatin1String(":message"), message);
    query.bindValue(QLatin1String(":status"), status);
    if (!query.exec()) {
        qWarning() << "Failed to write messageto DB:" << query.lastError().text();
        return;
    }

    // Write attachment
    if (attachment) {
        const QString queryString = QString(
            "INSERT INTO %1 (id, message_id, type, local_url, local_preview, size)"
            "VALUES (:id, :message_id, :type, :local_url, :local_preview, :size)"
        ).arg(m_attachments->tableName());
        QSqlQuery query;
        query.prepare(queryString);
        query.bindValue(QLatin1String(":id"), attachment->id);
        query.bindValue(QLatin1String(":message_id"), messageId);
        query.bindValue(QLatin1String(":type"), static_cast<int>(attachment->type));
        query.bindValue(QLatin1String(":local_url"), attachment->local_url.toString());
        query.bindValue(QLatin1String(":local_preview"), attachment->local_preview.toString());
        query.bindValue(QLatin1String(":size"), attachment->size);
        if (!query.exec()) {
            qWarning() << "Failed to write attachment to DB:" << query.lastError().text();
            return;
        }

    }
    resetModel(m_chatId);
}

void ConversationsModel::createMessage(const QString &messageId, const QString &message, const OptionalAttachment &attachment, const QString &contactId)
{
    writeMessage(messageId, message, attachment, contactId, StMessage::Author::User, EnMessageStatus::MST_CREATED);
}

void ConversationsModel::receiveMessage(const QString &messageId, const QString &message, const OptionalAttachment &attachment, const QString &contactId)
{
    writeMessage(messageId, message, attachment, contactId, StMessage::Author::Contact, EnMessageStatus::MST_RECEIVED);
}

QString ConversationsModel::user() const
{
    return m_userId;
}

void ConversationsModel::setUser(const QString &userId)
{
    if (m_userId == userId)
        return;

    m_userId = userId;
    m_tableName = QString("Conversations_") + Utils::escapedUserName(m_userId);

    // Create table
    createTable();
    m_attachments->createTable(userId);

    // Set query
    resetModel(QLatin1String());
}

void ConversationsModel::filterByChat(const QString &chatId)
{
    resetModel(chatId);
}

void ConversationsModel::markAsRead(const QString &chatId)
{
    const QString queryString = QString(
        "UPDATE %1 "
        "SET status = %2 "
        "WHERE chat_id = '%3' and status != %2"
    ).arg(m_tableName).arg(EnMessageStatus::MST_READ).arg(chatId);

    QSqlQuery query(queryString);
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to mark messages as read:") << query.lastError().text();
        return;
    }
    resetModel(m_chatId);
}

int ConversationsModel::getMessageCount(const QString &chatId, const EnMessageStatus status)
{
    const QString queryString = QString(
        "SELECT COUNT(1) as count FROM '%1' WHERE status = %2 AND chat_id = '%3'"
    ).arg(m_tableName).arg(status).arg(chatId);
    QSqlQuery query(queryString);
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to get message count:") << query.lastError().text();
        return 0;
    }
    if (query.next()) {
        return query.value(QLatin1String("count")).toInt();
    }
    return 0;
}

void ConversationsModel::setMessageStatus(const QString &messageId, const EnMessageStatus status)
{
    const QString queryString = QString(
        "UPDATE %1 "
        "SET status = %2 "
        "WHERE id = '%3'"
    ).arg(m_tableName).arg(status).arg(messageId);

    QSqlQuery query(queryString);
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to set message status:") << query.lastError().text();
        return;
    }
    resetModel(m_chatId);
}

std::vector<StMessage> ConversationsModel::getMessages(const QString &user, const EnMessageStatus status)
{
    std::vector<StMessage> messages;
    auto query = buildQuery(QLatin1String(), QString("status = %1").arg(status));
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to get messages:") << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        StMessage message;
        message.message = query.value(MessageColumn).toString();
        message.id = query.value(IdColumn).toString();
        message.author = query.value(AuthorColumn).value<StMessage::Author>();
        message.recipient = user;
        // TODO(fpohtmeh): add attachment
    }
    return messages;
}
