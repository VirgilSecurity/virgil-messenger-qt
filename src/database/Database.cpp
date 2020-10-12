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

#include "database/Database.h"

using namespace VSQ;

Database::Database(const QString &connectionName, const QString &connectionString)
    : m_connectionName(connectionName)
    , m_connectionString(connectionString)
{
}

Database::~Database()
{
    disconnect();
}

Database::Version Database::version() const
{
    return m_version;
}

bool Database::setup()
{
    return readVersion() && performMigration();
}

bool Database::connect()
{
    // TODO(fpohtmeh): implement
    return true;
}

bool Database::disconnect()
{
    // TODO(fpohtmeh): implement
    return true;
}

bool Database::startTransaction()
{
    // TODO(fpohtmeh): implement
    return true;
}

bool Database::commitTransaction()
{
    // TODO(fpohtmeh): implement
    return true;
}

bool Database::rollbackTransaction()
{
    // TODO(fpohtmeh): implement
    return true;
}

bool Database::tableExists(const QString &tableName) const
{
    for (auto &table : m_tables) {
        if (table->name() == tableName) {
            return true;
        }
    }
    return false;
}

bool Database::addTable(TablePointer table)
{
    if (tableExists(table->name())) {
        return true;
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

bool Database::readVersion()
{
    // TODO(fpohtmeh): implement
    return true;
}

bool Database::performMigration()
{
    // TODO(fpohtmeh): implement
    return true;
}
