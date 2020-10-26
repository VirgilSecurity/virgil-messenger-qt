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

#include <QSqlError>
#include <QSqlQuery>

#include "Utils.h"
#include "database/core/Database.h"

using namespace vm;

bool DatabaseUtils::isValidName(const QString &id)
{
    // TODO(fpohtmeh): add regexp check
    return !id.isEmpty();
}

QString DatabaseUtils::currentTimestamp()
{
    return QDateTime::currentDateTime().toString(Qt::ISODate);
}

Optional<QStringList> DatabaseUtils::readQueryTexts(const QString &filePath)
{
    const auto text = Utils::readTextFile(filePath);
    QStringList queries;
    const auto texts = text->split(";");
    for (auto text : texts) {
        auto query = text.trimmed();
        if (!query.isEmpty()) {
            queries << query;
        }
    }
    return queries;
}

bool DatabaseUtils::runQueries(Database *database, const QString &filePath)
{
    const auto texts = readQueryTexts(filePath);
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

Optional<QSqlQuery> DatabaseUtils::readBindQuery(Database *database, const QString &filePath, const BindValues &values)
{
    const auto text = Utils::readTextFile(filePath);
    if (!text) {
        return NullOptional;
    }
    auto query = database->createQuery();
    if (!query.prepare(*text)) {
        return NullOptional;
    }
    for (auto &v : values) {
        query.bindValue(v.first, v.second);
    }
    return query;
}

QString DatabaseUtils::resourcePath(const QString &fileName)
{
    return QString(":/resources/database/%1.sql").arg(fileName);
}

QString DatabaseUtils::errorText(const Optional<QSqlQuery> &query)
{
    return query ? query->lastError().databaseText() : QLatin1String("Query wasn't created");
}
