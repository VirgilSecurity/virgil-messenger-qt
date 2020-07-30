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

#include "database/AttachmentsModel.h"

#include <QSqlQuery>

#include "Settings.h"

AttachmentsModel::AttachmentsModel(Settings *settings, QObject *parent)
    : QSqlTableModel(parent)
    , m_settings(settings)
{}

bool AttachmentsModel::createTable(const QString &user)
{
    const QString attachmentsTableName = QLatin1String("Attachments_") + Utils::escapedUserName(user);
    const QString conversationsTableName = QLatin1String("Conversations_") + Utils::escapedUserName(user);
    const QString queryText = QString(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "  id TEXT NOT NULL,"
        "  message_id TEXT NOT NULL,"
        "  type INTEGER NOT NULL,"
        "  remote_url TEXT,"
        "  local_url TEXT,"
        "  local_preview TEXT,"
        "  size INTEGER NOT NULL,"
        "  FOREIGN KEY(message_id) REFERENCES %2 (id)"
        ")"
    ).arg(attachmentsTableName, conversationsTableName);
    QSqlQuery query(queryText);
    if (!query.exec()) {
        qCritical() << (QString("Failed to create database: %1").arg(attachmentsTableName));
        return false;
    }

    setTable(attachmentsTableName);
    return true;
}
