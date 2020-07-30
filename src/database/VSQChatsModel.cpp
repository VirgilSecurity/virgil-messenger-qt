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

#include "models/ChatsModel.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "models/ContactsModel.h"
#include "Utils.h"

ChatsModel::ChatsModel(ContactsModel *contacts, QObject *parent)
    : QSqlTableModel(parent)
    , m_contacts(contacts)
{}

void ChatsModel::setUser(const QString &userId)
{
    if (userId == m_userId)
        return;
    qDebug() << "Initialize chats with user: " << userId;
    m_userId = userId;

    // Initialize contacts
    m_contacts->setUser(userId);

    // Create table
    m_tableName = QLatin1String("Chats_") + Utils::escapedUserName(userId);
    const QString queryString =
        "CREATE TABLE IF NOT EXISTS '%1' ("
        "  id TEXT NOT NULL UNIQUE,"
        "  contact_id TEXT NOT NULL,"
        "  last_message TEXT,"
        "  last_message_time TEXT,"
        "  unread_message_count INTEGER NOT NULL,"
        "  FOREIGN KEY(contact_id) REFERENCES %2 (id)"
        ")";
    QSqlQuery query(queryString.arg(m_tableName).arg(m_contacts->tableName()));
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to create chats table:") << query.lastError().text();
        return;
    }

    // Set model table
    setTable(m_tableName);
    setEditStrategy(EditStrategy::OnRowChange);
    setSort(LastMessageTimeColumn, Qt::DescendingOrder);
    select();
}

QVariant ChatsModel::data(const QModelIndex &index, int role) const
{
    if (role < Qt::UserRole) {
        return QSqlTableModel::data(index, role);
    }
    return record(index.row()).value(role - Qt::UserRole);
}

QHash<int, QByteArray> ChatsModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::UserRole + IdColumn] = "id";
    names[Qt::UserRole + ContactIdColumn] = "contact";
    names[Qt::UserRole + LastMessageColumn] = "lastMessage";
    names[Qt::UserRole + LastMessageTimeColumn] = "lastMessageTime";
    names[Qt::UserRole + UnreadMessagesCountRole] = "unreadMessageCount";
    return names;
}

void ChatsModel::clearFilter()
{
    setFilter(QLatin1String());
}

void ChatsModel::applyFilter(const QString &filter)
{
    setFilter(QString("contact_id LIKE '%%1%'").arg(filter));
}

Optional<QString> ChatsModel::createPrivateChat(const QString &contactId)
{
    m_contacts->create(contactId);

    const QString queryString = QString(
        "SELECT * FROM '%1' WHERE contact_id = '%2'"
    ).arg(m_tableName, contactId);
    QSqlQuery query(queryString);
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to find a chat:") << query.lastError().text();
        return NullOptional;
    }
    if (query.next()) {
        qInfo() << QString("Chat with %1 already exists").arg(contactId);
        return query.value(IdColumn).toString();
    }

    qDebug() << "Create private chat with: " << contactId;
    QSqlRecord newRecord = record();
    const auto id = Utils::createUuid();
    newRecord.setValue(IdColumn, id);
    newRecord.setValue(ContactIdColumn, contactId);
    newRecord.setValue(UnreadMessagesCountRole, 0);
    newRecord.setValue(LastMessageTimeColumn, Utils::currentIsoDateTime()); // for sorting
    if (!insertRowIntoTable(newRecord)) {
        qCritical() << QLatin1String("Failed to create private chat:") << lastError().text();
        return NullOptional;
    }

    submit();
    select();
    return id;
}

void ChatsModel::updateLastMessage(const QString &contactId, const QString &message)
{
    const QString queryString = QString(
        "UPDATE '%1' "
        "SET last_message = :last_message, last_message_time = :last_message_time "
        "WHERE contact_id = '%2'"
    ).arg(m_tableName, contactId);

    QSqlQuery query;
    query.prepare(queryString);
    query.bindValue(QLatin1String(":last_message"), message);
    query.bindValue(QLatin1String(":last_message_time"), Utils::currentIsoDateTime());
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to update last message:") << query.lastError().text();
        return;
    }
    submit();
    select();
}

void ChatsModel::setUnreadMessageCount(const QString &chatId, int count)
{
    const QString queryString = QString(
        "UPDATE '%1' "
        "SET unread_message_count = '%2' "
        "WHERE id = '%3'"
    ).arg(m_tableName).arg(count).arg(chatId);
    QSqlQuery query(queryString);
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to set unread message count:") << query.lastError().text();
        return;
    }
    submit();
    select();
}
