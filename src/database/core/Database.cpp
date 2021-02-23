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

#include "database/core/Database.h"

#include <QSqlError>
#include <QLoggingCategory>

using namespace vm;
using Self = Database;

Q_LOGGING_CATEGORY(lcDatabase, "db")

Self::Database(const Version &latestVersion, QObject *parent) : QObject(parent), m_latestVersion(latestVersion)
{
    qCDebug(lcDatabase) << "Database latest version:" << latestVersion;
}

Self::~Database()
{
    close();
}

bool Self::open(const QString &databaseFileName, const QString &connectionName)
{
    close();

    qCDebug(lcDatabase) << "Opening of database:" << databaseFileName;
    m_qtDatabase = QSqlDatabase::addDatabase(m_type, connectionName);
    m_qtDatabase.setDatabaseName(databaseFileName);

    ScopedConnection connection(*this);
    // Read version
    if (!readVersion()) {
        return false;
    }
    // Create tables
    {
        ScopedTransaction transaction(*this);
        if (!create()) {
            emit errorOccurred(tr("Failed to create database"));
            qCCritical(lcDatabase) << "Self::create error";
            return transaction.rollback();
        }
    }
    // Ignore downgrade
    if (m_latestVersion < m_version) {
        qCWarning(lcDatabase) << "Database can't be downgraded";
    }
    // Perform migration (upgrade)
    else if (m_latestVersion > m_version) {
        qCDebug(lcDatabase) << "Database migration:" << m_version << "=>" << m_latestVersion;
        ScopedTransaction transaction(*this);
        if (m_migration && !m_migration->run(this)) {
            emit errorOccurred(tr("Database migration failed"));
            qCCritical(lcDatabase) << "Database migration error";
            return transaction.rollback();
        }
        if (!writeVersion()) {
            return transaction.rollback();
        }
    }
    qCDebug(lcDatabase) << "Database was opened";
    emit opened();
    return true;
}

void Self::close()
{
    const auto connectionName = m_qtDatabase.connectionName();
    if (!QSqlDatabase::contains(connectionName)) {
        return;
    }
    m_qtDatabase = {};
    QSqlDatabase::removeDatabase(connectionName);
    qCDebug(lcDatabase) << "Database was closed";
    emit closed();
}

QSqlQuery Self::createQuery() const
{
    return QSqlQuery(m_qtDatabase);
}

bool Self::tableExists(const QString &tableName) const
{
    return m_qtDatabase.tables().contains(tableName);
}

bool Self::addTable(TablePointer table)
{
    if (!tableExists(table->name()) && !table->create()) {
        return false;
    }
    m_tables.push_back(std::move(table));
    return true;
}

const Self::Tables &Self::tables() const
{
    return m_tables;
}

Self::Tables &Self::tables()
{
    return m_tables;
}

DatabaseTable *Self::table(int index)
{
    return m_tables[index].get();
}

const DatabaseTable *Self::table(int index) const
{
    return m_tables[index].get();
}

Self::Version Self::version() const
{
    return m_version;
}

void Self::setMigration(std::unique_ptr<Migration> migration)
{
    m_migration = std::move(migration);
}

Self::operator QSqlDatabase() const
{
    return m_qtDatabase;
}

bool Self::create()
{
    return true;
}

bool Self::readVersion()
{
    auto query = createQuery();
    const auto queryText = QLatin1String("PRAGMA user_version;");
    if (!query.exec(queryText)) {
        emit errorOccurred(tr("Failed to read database version"));
        qCCritical(lcDatabase) << "Self::readVersion error";
        return false;
    }
    m_version = query.next() ? query.value(0).toULongLong() : 0;
    qCDebug(lcDatabase) << "Database version:" << m_version;
    return true;
}

bool Self::writeVersion()
{
    auto query = createQuery();
    const auto queryText = QString("PRAGMA user_version(%1);").arg(m_latestVersion);
    if (!query.exec(queryText)) {
        emit errorOccurred(tr("Failed to write database version"));
        qCCritical(lcDatabase) << "Self::writeVersion error";
        return false;
    }
    m_version = m_latestVersion;
    qCDebug(lcDatabase) << "Database version was updated:" << m_latestVersion;
    return true;
}
