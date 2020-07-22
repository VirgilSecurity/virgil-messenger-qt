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

#include "models/ContactsModel.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "Utils.h"

ContactsModel::ContactsModel(QObject *parent)
    : QSqlTableModel(parent)
{}

void ContactsModel::setUser(const QString &userId)
{
    if (m_userId == userId)
        return;
    qDebug() << "Initialize contacts with user: " << userId;
    m_userId = userId;

    m_tableName = QLatin1String("Contacts_") + Utils::escapedUserName(userId);
    const QString queryString = QString(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "  id TEXT NOT NULL PRIMARY KEY"
        ")"
    ).arg(m_tableName);
    QSqlQuery query(queryString);
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to create contacts table:") << query.lastError().text();
    }

    setTable(m_tableName);
    setEditStrategy(EditStrategy::OnRowChange);
    select();
}

Optional<QString> ContactsModel::create(const QString &id)
{
    const QString queryString = QString(
        "SELECT id "
        "FROM %1 "
        "WHERE id = '%2'"
    ).arg(m_tableName, id);
    QSqlQuery query(queryString);
    if (!query.exec()) {
        qCritical() << QLatin1String("Failed to find contact:") << query.lastError().text();
        return NullOptional;
    }
    if (query.next()) {
        qInfo() << QString("Contact %1 already exists").arg(id);
        return id;
    }

    QSqlRecord newRecord = record();
    newRecord.setValue(IdColumn, id);
    if (!insertRowIntoTable(newRecord)) {
        qCritical() << QLatin1String("Failed to add contact:") << lastError().text();
        return NullOptional;
    }
    return id;
}
