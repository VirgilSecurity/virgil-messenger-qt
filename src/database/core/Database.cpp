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

#include "database/core/ConnectionScope.h"
#include "database/core/TransactionScope.h"

Q_LOGGING_CATEGORY(lcDatabase, "database")

using namespace VSQ;

Database::Database(const Version &version)
    : m_version(version)
{
}

Database::~Database()
{
    close();
}

bool Database::open(const QString &databaseFileName, const QString &connectionName)
{
    close();

    qCDebug(lcDatabase) << "Opening of database:" << databaseFileName;
    m_connectionName = connectionName;
    m_connection = QSqlDatabase::addDatabase(m_type, m_connectionName);
    m_connection.setDatabaseName(databaseFileName);

    ConnectionScope connection(this);
    {
        TransactionScope transaction(this);
        if (!transaction.addResult(create())) {
            return false;
        }
    }
    if (!readVersion()) {
        return false;
    }
    if (m_version > m_databaseVersion) {
        TransactionScope transaction(this);
        return transaction.addResult(performMigration() && writeVersion());
    }
    return true;
}

bool Database::openConnection()
{
    if (!m_connection.open()) {
        qCCritical(lcDatabase) << "Connection databaseName:" << m_connection.databaseName();
        qCCritical(lcDatabase) << "Connection error:" << m_connection.lastError().databaseText();
        return false;
    }
    return true;
}

void Database::closeConnection()
{
    m_connection.close();
}

QSqlQuery Database::createQuery() const
{
    return QSqlQuery(m_connection);
}

bool Database::startTransaction()
{
    return m_connection.transaction();
}

bool Database::commitTransaction()
{
    return m_connection.commit();
}

bool Database::rollbackTransaction()
{
    return m_connection.rollback();
}

bool Database::tableExists(const QString &tableName) const
{
    return m_connection.tables().contains(tableName);
}

bool Database::addTable(TablePointer table)
{
    if (tableExists(table->name())) {
        return false;
    }
    if (!table->create(this)) {
        return false;
    }
    m_tables.push_back(std::move(table));
    return true;
}

const Database::Tables &Database::tables() const
{
    return m_tables;
}

Database::Tables &Database::tables()
{
    return m_tables;
}

DatabaseTable *Database::table(int index)
{
    return m_tables[index].get();
}

const DatabaseTable *Database::table(int index) const
{
    return m_tables[index].get();
}

bool Database::create()
{
    return true;
}

bool Database::performMigration()
{
    return true;
}

void Database::close()
{
    if (m_connection.isOpen()) {
        closeConnection();
    }
    if (!m_connectionName.isEmpty()) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool Database::readVersion()
{
    // TODO(fpohtmeh): read to m_databaseVersion
    return true;
}

bool Database::writeVersion()
{
    // TODO(fpohtmeh): write version to database
    return true;
}
