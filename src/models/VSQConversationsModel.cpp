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

#include "models/VSQConversationsModel.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

#include "VSQUtils.h"
#include "models/VSQAttachmentsModel.h"
#include "models/VSQChatsModel.h"

Q_DECLARE_METATYPE(EnMessageStatus) // TODO(fpohtmeh): move to application

VSQConversationsModel::VSQConversationsModel(VSQChatsModel *chat, VSQAttachmentsModel *attachments, QObject *parent)
    : QSqlQueryModel(parent)
    , m_chat(chat)
    , m_attachments(attachments)
{
    qRegisterMetaType<EnMessageStatus>("EnMessageStatus");
}

QVariant VSQConversationsModel::data(const QModelIndex &index, int role) const
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
    else {
        return currRecord.value(column);
    }
}

void VSQConversationsModel::createTable()
{
    const QString queryString = QString(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "  author INTEGER NOT NULL,"
        "  chat_id TEXT NOT NULL,"
        "  timestamp TEXT NOT NULL,"
        "  message TEXT NOT NULL,"
        "  status INTEGER NOT NULL,"
        "  message_id TEXT NOT NULL"
        ")"
    );
    QSqlQuery query(queryString.arg(m_tableName));
    if (!query.exec()) {
        qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
    }
    QSqlQuery indexQuery;
    if (!indexQuery.exec(
        QString("CREATE UNIQUE INDEX IF NOT EXISTS idx_%1_message_id ON %1 (message_id);").arg(m_tableName))) {
        qFatal("Failed to query database: %s", qPrintable(indexQuery.lastError().text()));
    }
}

void VSQConversationsModel::resetModel(const QString &chatId)
{
    m_chatId = chatId;
    setQuery(buildQuery(chatId, QLatin1String()));
}

QSqlQuery VSQConversationsModel::buildQuery(const QString &chatId, const QString &condition) const
{
    const QString main = QString("SELECT * FROM %1 ").arg(m_tableName);
    QStringList whereList;
    if (!chatId.isEmpty())
        whereList << QString("chat_id = '%1'").arg(chatId);
    if (!condition.isEmpty())
        whereList << condition;

    QString queryString = main;
    if (!whereList.isEmpty())
        queryString.append(QLatin1String(" WHERE ") + whereList.join(QLatin1String(" AND ")));
    return QSqlQuery(queryString);
}

QHash<int, QByteArray> VSQConversationsModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::UserRole + AuthorColumn] = "author";
    names[Qt::UserRole + ChatIdColumn] = "chat_id";
    names[Qt::UserRole + TimestampColumn] = "timestamp";
    names[Qt::UserRole + MessageColumn] = "message";
    names[Qt::UserRole + StatusColumn] = "status";
    names[Qt::UserRole + MessageIdColumn] = "message_id";
    names[Qt::UserRole + FirstMessageInARowColumn] = "firstMessageInARow";
    names[Qt::UserRole + MessageInARowColumn] = "messageInARow";
    names[Qt::UserRole + DayColumn] = "day";
    return names;
}

void VSQConversationsModel::writeMessage(const QString &messageId, const QString &message, const QString &recipientId,
                                        const StMessage::Author author, const EnMessageStatus status)
{
    const auto chatId = m_chat->createPrivateChat(recipientId);
    const QString queryString = QString(
        "INSERT INTO %1 (author, chat_id, timestamp, message, status, message_id)"
        "VALUES (:author, :chat_id, :timestamp, :message, :status, :message_id)"
    ).arg(m_tableName);
    QSqlQuery query;
    query.prepare(queryString);
    query.bindValue(QLatin1String(":author"), static_cast<int>(author));
    query.bindValue(QLatin1String(":chat_id"), *chatId);
    query.bindValue(QLatin1String(":timestamp"), Utils::currentIsoDateTime());
    query.bindValue(QLatin1String(":message"), message);
    query.bindValue(QLatin1String(":status"), status);
    query.bindValue(QLatin1String(":message_id"), messageId);
    if (!query.exec()) {
        qWarning() << "Failed to save received message:" << query.lastError().text();
        return;
    }
    resetModel(m_chatId);
}

void VSQConversationsModel::createMessage(const QString &messageId, const QString &message, const QString &contactId)
{
    writeMessage(messageId, message, contactId, StMessage::Author::User, EnMessageStatus::MST_CREATED);
}

void VSQConversationsModel::receiveMessage(const QString &messageId, const QString &message, const QString &contactId)
{
    writeMessage(messageId, message, contactId, StMessage::Author::Contact, EnMessageStatus::MST_RECEIVED);
}

QString VSQConversationsModel::user() const
{
    return m_userId;
}

void VSQConversationsModel::setUser(const QString &userId)
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

void VSQConversationsModel::filterByChat(const QString &chatId)
{
    resetModel(chatId);
}

void VSQConversationsModel::markAsRead(const QString &chatId)
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

int VSQConversationsModel::getMessageCount(const QString &chatId, const EnMessageStatus status)
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

void VSQConversationsModel::setMessageStatus(const QString &messageId, const EnMessageStatus status)
{
    const QString queryString = QString(
        "UPDATE %1 "
        "SET status = %2 "
        "WHERE message_id = '%3'"
    ).arg(m_tableName).arg(status).arg(messageId);

    QSqlQuery query(queryString);
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to set message status:") << query.lastError().text();
        return;
    }
    resetModel(m_chatId);
}

std::vector<StMessage> VSQConversationsModel::getMessages(const QString &user, const EnMessageStatus status)
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
        message.message_id = query.value(MessageIdColumn).toString();
        message.author = query.value(AuthorColumn).value<StMessage::Author>();
        message.recipient = user;
        // TODO(fpohtmeh): add attachment
    }
    return messages;
}
