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

#include "VSQSqlChatModel.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDateTime>

#include "VSQSqlConversationModel.h"

/******************************************************************************/
VSQSqlChatModel::VSQSqlChatModel(QObject *parent) :
    QSqlTableModel(parent) {
}

/******************************************************************************/
void
VSQSqlChatModel::init(const QString &userId) {

    qDebug() << "Initialize chat model with user: " << userId;

    if (userId == m_userId) {
        return;
    }

    m_userId = userId;
    m_tableName = "Chats_" + userId;

    QString queryString =
            "CREATE TABLE IF NOT EXISTS '%1' ("
            "  'id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
            "  'name' TEXT NOT NULL,"
            "  'last_message' TEXT,"
            "  'last_message_time' TEXT,"
            "  'unread_message_count' INTEGER NOT NULL"
            ")";

    QSqlQuery query(queryString.arg(m_tableName));
    if (!query.exec()) {
        qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
    }

    setTable(m_tableName);
    setEditStrategy(EditStrategy::OnRowChange);
    setSort(3, Qt::DescendingOrder); // last_message_time
    select();
}

/******************************************************************************/
QVariant
VSQSqlChatModel::data(const QModelIndex &index, int role) const {
    if (role < Qt::UserRole) {
        return QSqlTableModel::data(index, role);
   }

   const QSqlRecord currRecord = record(index.row());
   return currRecord.value(role - Qt::UserRole);
}

/******************************************************************************/
QHash<int, QByteArray>
VSQSqlChatModel::roleNames() const {
    QHash<int, QByteArray> names;
    names[Qt::UserRole] = "id";
    names[Qt::UserRole + 1] = "name";
    names[Qt::UserRole + 2] = "lastMessage";
    names[Qt::UserRole + 3] = "lastMessageTime";
    names[Qt::UserRole + 4] = "unreadMessageCount";

    return names;
}

/******************************************************************************/
void
VSQSqlChatModel::clearFilter() {
    setFilter("");
}

/******************************************************************************/
void
VSQSqlChatModel::applyFilter(const QString &filter) {

    const QString filterString = QString::fromLatin1(
        "name LIKE '%%1%'").arg(filter);

    setFilter(filterString);
}

/******************************************************************************/
void
VSQSqlChatModel::createPrivateChat(const QString &recipientId) {

    qDebug() << "Create private chat with: " << recipientId;

    QSqlQueryModel selectModel;
    QString selectQuery =
            "SELECT COUNT(*) AS 'count' "
            "  FROM '%1' "
            " WHERE name = '%2'";

    QSqlQuery query1(selectQuery.arg(m_tableName, recipientId));
    selectModel.setQuery(query1);
    int count = selectModel.record(0).value("count").toInt();

    if (count > 0) {
        return;
    }

    QSqlRecord newRecord = record();
    newRecord.setValue("name", recipientId);
    newRecord.setValue("unread_message_count", 0);

    if (!insertRowIntoTable(newRecord)) {
        qWarning() << "Failed to send message:" << lastError().text();
        return;
    }

    submitAll();
    select();
}

/******************************************************************************/
void VSQSqlChatModel::updateLastMessage(QString chatId, QString message) {

    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

    QString updateQuery =
            "UPDATE '%1'"
            "   SET last_message = ?,"
            "       last_message_time = ?"
            " WHERE "
            "       name = '%2'";

    QSqlQuery query(updateQuery.arg(m_tableName, chatId));

    query.bindValue(0, message);
    query.bindValue(1, timestamp);

    if (!query.exec()) {
        qDebug() << query.lastQuery();
        qFatal("Failed to update last message: %s", qPrintable(query.lastError().text()));
    }

    submitAll();
    select();
}

/******************************************************************************/
void VSQSqlChatModel::updateUnreadMessageCount(QString chatId) {
    QSqlQueryModel selectModel;
    QString selectQuery =
            "SELECT COUNT(*) AS 'count' "
            "  FROM '%1' "
            " WHERE status = %3 AND author = '%2'";

    QSqlQuery query1(selectQuery
                     .arg("Conversations_" + m_userId, chatId)
                     .arg(EnMessageStatus::MST_RECEIVED));
    selectModel.setQuery(query1);
    int count = selectModel.record(0).value("count").toInt();

    QString updateQuery =
            "UPDATE '%1'"
            "   SET unread_message_count = '%2'"
            " WHERE "
            "       name = '%3'";

    QSqlQuery query(updateQuery.arg(m_tableName, QString::number(count), chatId));

    if (!query.exec()) {
        qDebug() << query.lastQuery();
        qFatal("Failed to update unread message count: %s", qPrintable(query.lastError().text()));
    }

    submitAll();
    select();
}
