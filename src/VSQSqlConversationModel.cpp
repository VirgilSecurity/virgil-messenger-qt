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

#include "VSQSqlConversationModel.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

/******************************************************************************/
void
VSQSqlConversationModel::_createTable() {
    QSqlQuery query;
    if (!query.exec(
        QString("CREATE TABLE IF NOT EXISTS %1 ("
        "'author' TEXT NOT NULL,"
        "'recipient' TEXT NOT NULL,"
        "'timestamp' TEXT NOT NULL,"
        "'message' TEXT NOT NULL,"
        "'is_read' BOOL NOT NULL,"
        "FOREIGN KEY('author') REFERENCES %2 ( name ),"
        "FOREIGN KEY('recipient') REFERENCES %3 ( name )"
        ")")
                .arg(_tableName())
                .arg(_contactsTableName())
                .arg(_contactsTableName()))) {
        qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
    }
}

/******************************************************************************/
void
VSQSqlConversationModel::_update() {
    setTable(_tableName());
    setSort(2, Qt::DescendingOrder);
    // Ensures that the model is sorted correctly after submitting a new row.
    setEditStrategy(QSqlTableModel::OnManualSubmit);

    setFilter("(recipient = '')");
    select();

    emit recipientChanged();
}

/******************************************************************************/
VSQSqlConversationModel::VSQSqlConversationModel(QObject *parent) :
    QSqlTableModel(parent) {
}

/******************************************************************************/
QString
VSQSqlConversationModel::recipient() const {
    return m_recipient;
}

/******************************************************************************/
void
VSQSqlConversationModel::setRecipient(const QString &recipient) {
    if (recipient == m_recipient) {
        return;
    }

    // TODO: Prevent SQL injection !!!
    m_recipient = recipient;

    const QString filterString = QString::fromLatin1(
        "(recipient = '%1' AND author = '%2') OR (recipient = '%2' AND author='%1')").arg(m_recipient, user());

    setSort(2, Qt::AscendingOrder);
    setFilter(filterString);

        // select();

    emit recipientChanged();
}

/******************************************************************************/
QVariant
VSQSqlConversationModel::data(const QModelIndex &index, int role) const {
    if (role < Qt::UserRole) {
        return QSqlTableModel::data(index, role);
   }

    const int firstMessageInARow = Qt::UserRole + 4;
    const int messageInARow = Qt::UserRole + 5;
    const int day = Qt::UserRole + 6;

    const QSqlRecord currRecord = record(index.row());

    if (role == firstMessageInARow) {
        const QSqlRecord prevRecord = record(index.row() - 1);
        const QVariant prevMsgAuthor = prevRecord.value(0);
        const QVariant currMsgAuthor = currRecord.value(0);
        const QVariant prevTimestamp = prevRecord.value(2);
        const QVariant currTimestamp = currRecord.value(2);

        // Check if previous message is from the same author
        const bool isAuthor = currMsgAuthor.toString() != prevMsgAuthor.toString();

        // Check if the message was sent in last 5 min
        const bool isInFiveMinRange = prevTimestamp.toDateTime().addSecs(5 * 60) > currTimestamp.toDateTime();

        // Message is considered to be first in a row when it
        // sends in a range of 1 min with previous message and
        // from the same author
        return isAuthor || !isInFiveMinRange;
    }

    if (role == messageInARow) {
        const QSqlRecord nextRecord = record(index.row() + 1);
        const QVariant nextMsgAuthor = nextRecord.value(0);
        const QVariant currMsgAuthor = currRecord.value(0);

        return currMsgAuthor.toString() == nextMsgAuthor.toString();
    }

    if (role == day) {
        const QVariant timestamp = currRecord.value(2);
        return timestamp.toDate();
    }

    return currRecord.value(role - Qt::UserRole);
}

/******************************************************************************/
QHash<int, QByteArray>
VSQSqlConversationModel::roleNames() const {
    QHash<int, QByteArray> names;
    names[Qt::UserRole] = "author";
    names[Qt::UserRole + 1] = "recipient";
    names[Qt::UserRole + 2] = "timestamp";
    names[Qt::UserRole + 3] = "message";
    names[Qt::UserRole + 4] = "firstMessageInARow";
    names[Qt::UserRole + 5] = "messageInARow";
    names[Qt::UserRole + 6] = "day";
    return names;
}

/******************************************************************************/
void
VSQSqlConversationModel::sendMessage(QString recipient, QString message) {
    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

    QSqlRecord newRecord = record();
    newRecord.setValue("author", user());
    newRecord.setValue("recipient", recipient);
    newRecord.setValue("timestamp", timestamp);
    newRecord.setValue("message", message);
    newRecord.setValue("is_read", false);
    if (!insertRecord(rowCount(), newRecord)) {
        qWarning() << "Failed to send message:" << lastError().text();
        return;
    }

    submitAll();
}

/******************************************************************************/
void
VSQSqlConversationModel::receiveMessage(const QString &sender, const QString &message) {
    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

    QSqlRecord newRecord = record();
    newRecord.setValue("author", sender);
    newRecord.setValue("recipient", user());
    newRecord.setValue("timestamp", timestamp);
    newRecord.setValue("message", message);
    if (!insertRecord(rowCount(), newRecord)) {
        qWarning() << "Failed to save received message:" << lastError().text();
        return;
    }

    submitAll();
}

/******************************************************************************/
QString
VSQSqlConversationModel::user() const {
    return m_user;
}

/******************************************************************************/
void
VSQSqlConversationModel::setUser(const QString &user) {
    m_recipient = "";
    if (user == m_user) {
        return;
    }

    m_user = user;

    _createTable();
    _update();
}

/******************************************************************************/
int
VSQSqlConversationModel::getCountOfUnread(const QString &user) {
    QSqlQueryModel model;
    QString query;

    query = QString("SELECT COUNT(*) AS C FROM %1 WHERE is_read = 0 AND recipient = \"%2\"").arg(_tableName()).arg(user);

    model.setQuery(query);
    int c = model.record(0).value("C").toInt();

    qDebug() << c << user << query;

    return c;
}

/******************************************************************************/
QString
VSQSqlConversationModel::getLastMessage(const QString &user) const {
    QSqlQueryModel model;
    QString query;

    query = QString("SELECT * FROM %1 WHERE recipient = \"%2\" ORDER BY timestamp DESC LIMIT 1").arg(_tableName()).arg(user);

    model.setQuery(query);
    QString message = model.record(0).value("message").toString();

    qDebug() << message << user << query;

    return message;
}

/******************************************************************************/
void
VSQSqlConversationModel::setAsRead(const QString &user) {
    QSqlQuery model;
    QString query;

    query = QString("UPDATE %1 SET is_read = 1 WHERE recipient = \"%2\"").arg(_tableName()).arg(user);

    model.prepare(query);

    qDebug() << user << query << model.exec();
}

/******************************************************************************/
QString
VSQSqlConversationModel::getLastMessageTime(const QString &user) const {
    QSqlQueryModel model;
    QString query;

    query = QString("SELECT * FROM %1 WHERE recipient = \"%2\" ORDER BY timestamp DESC LIMIT 1").arg(_tableName()).arg(user);

    model.setQuery(query);
    QString timestamp = model.record(0).value("timestamp").toString();

    qDebug() << timestamp << user << query;

    return timestamp;
}

/******************************************************************************/
QString
VSQSqlConversationModel::_tableName() const {
    QString fixedUser(m_user);
    fixedUser.remove(QRegExp("[^a-zA-Z\\d\\s]"));
    return QString("Conversations_") + fixedUser;
}

/******************************************************************************/
QString
VSQSqlConversationModel::_contactsTableName() const {
    QString fixedUser(m_user);
    fixedUser.remove(QRegExp("[^a-zA-Z\\d\\s]"));
    return QString("Contacts_") + fixedUser;
}

/******************************************************************************/
