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

#include "database/ContactsTable.h"

#include "database/core/Database.h"
#include "database/core/DatabaseUtils.h"

using namespace vm;
using Self = ContactsTable;

Self::ContactsTable(Database *database)
    : DatabaseTable(QLatin1String("contacts"), database)
{
    connect(this, &Self::addContact, this, &Self::onAddContact);
}

bool Self::create()
{
    if (DatabaseUtils::readExecQueries(database(), QLatin1String("createContacts"))) {
        qCDebug(lcDatabase) << "Contacts table was created";
        return true;
    }
    qCCritical(lcDatabase) << "Failed to create contacts table";
    return false;
}


void Self::onAddContact(const Contact& contact) {

    DatabaseUtils::BindValues bindValues{
        { ":userId", QString(contact.userId()) },
        { ":username", contact.username() },
        { ":name", contact.name() },
        { ":email", contact.email() },
        { ":phone", contact.phone() },
        { ":platformId", contact.platformId() },
        { ":avatarLocalPath", contact.avatarLocalPath() },
        { ":isBanned", contact.isBanned() }
    };

    ScopedConnection connection(*database());
    const auto query = DatabaseUtils::readExecQuery(database(), "insertContact", bindValues);
    if (query) {
        qCDebug(lcDatabase) << "Contact was inserted, user id:" << bindValues.front().second;
    }
    else {
        qCWarning(lcDatabase) << "Contact was not inserted";
    }
}
