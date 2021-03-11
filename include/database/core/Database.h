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

#ifndef VM_DATABASE_H
#define VM_DATABASE_H

#include "DatabaseTable.h"
#include "Migration.h"
#include "ScopedConnection.h"
#include "ScopedTransaction.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcDatabase);

namespace vm {
class Database : public QObject
{
    Q_OBJECT

public:
    using Version = Patch::Version;
    using TablePointer = std::unique_ptr<DatabaseTable>;
    using Tables = std::vector<TablePointer>;

    Database(const Version &latestVersion, QObject *parent);
    virtual ~Database();

    bool open(const QString &databaseFileName, const QString &connectionName);
    void close();
    QSqlQuery createQuery() const;

    bool tableExists(const QString &tableName) const;
    bool addTable(TablePointer table);
    const Tables &tables() const;
    Tables &tables();
    DatabaseTable *table(int index);
    const DatabaseTable *table(int index) const;

    Version version() const;
    void setMigration(std::unique_ptr<Migration> migration);

    //
    //  Proxy to sqlite3_changes().
    //
    qsizetype changes() const;

    operator QSqlDatabase() const;

signals:
    void opened();
    void closed();
    void errorOccurred(const QString &errorText);

protected:
    virtual bool create();

private:
    bool readVersion();
    bool writeVersion();

    const QLatin1String m_type = QLatin1String("QSQLITE");
    const Version m_latestVersion = 0;
    Version m_version = 0;
    std::unique_ptr<Migration> m_migration;
    QSqlDatabase m_qtDatabase;
    Tables m_tables;
};
} // namespace vm

#endif // VM_DATABASE_H
