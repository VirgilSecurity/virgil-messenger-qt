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

#ifndef VSQ_DATABASE_H
#define VSQ_DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>

#include "DatabaseTable.h"
#include "Migration.h"

Q_DECLARE_LOGGING_CATEGORY(lcDatabase)

namespace VSQ
{
class Database
{
    Q_DISABLE_COPY(Database)

public:
    using Version = Patch::Version;
    using TablePointer = std::unique_ptr<DatabaseTable>;
    using Tables = std::vector<TablePointer>;

    explicit Database(const Version &latestVersion);
    virtual ~Database();

    bool open(const QString &databaseFileName, const QString &connectionName);

    bool openConnection();
    void closeConnection();

    QSqlQuery createQuery() const;

    bool startTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    bool tableExists(const QString &tableName) const;
    bool addTable(TablePointer table);
    const Tables &tables() const;
    Tables &tables();
    DatabaseTable *table(int index);
    const DatabaseTable *table(int index) const;

    Version version() const;
    void setMigration(std::unique_ptr<Migration> migration);

protected:
    virtual bool create();

private:
    void close();

    bool readVersion();
    bool writeVersion();

    const QLatin1String m_type = QLatin1String("QSQLITE");
    const Version m_latestVersion = 0;
    QString m_connectionName;
    Version m_version = 0;
    std::unique_ptr<Migration> m_migration;
    QSqlDatabase m_connection;
    Tables m_tables;
};
}

#endif // VSQ_DATABASE_H
