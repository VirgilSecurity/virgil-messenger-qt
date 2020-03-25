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

#include "VSQSqlContactModel.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

/******************************************************************************/
void
VSQSqlContactModel::_createTable() {
    if (QSqlDatabase::database().tables().contains(QStringLiteral("Contacts"))) {
        // The table already exists; we don't need to do anything.
        return;
    }

    QSqlQuery query;
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS 'Contacts' ("
        "   'name' TEXT NOT NULL,"
        "   PRIMARY KEY(name)"
        ")")) {
        qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
    }
}

/******************************************************************************/
VSQSqlContactModel::VSQSqlContactModel(QObject *parent) :
    QSqlQueryModel(parent) {
    _createTable();
    _update();
}

/******************************************************************************/
void
VSQSqlContactModel::_update() {
    QSqlQuery query;
    if (!query.exec("SELECT * FROM Contacts")) {
        qFatal("Contacts SELECT query failed: %s", qPrintable(query.lastError().text()));
    }

    setQuery(query);
    if (lastError().isValid()) {
        qFatal("Cannot set query on SqlContactModel: %s", qPrintable(lastError().text()));
    }
}

/******************************************************************************/
void
VSQSqlContactModel::addContact(QString contact) {
    QSqlQuery query;
    query.prepare("INSERT OR IGNORE INTO Contacts VALUES (:contact)");
    query.bindValue(":contact", contact);
    query.exec();

    if (lastError().isValid()) {
        qWarning("Cannot set query on SqlContactModel: %s", qPrintable(lastError().text()));
    }

    _update();
}

/******************************************************************************/
