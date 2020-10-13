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

#include "database/UserDatabase.h"

#include "VSQSettings.h"
#include "database/AttachmentsTable.h"
#include "database/ChatsTable.h"
#include "database/ContactsTable.h"
#include "database/MessagesTable.h"

using namespace VSQ;

UserDatabase::UserDatabase(const VSQSettings *settings)
    : Database(DATABASE_SCHEME_VERSION)
    , m_settings(settings)
{}

UserDatabase::~UserDatabase()
{}

bool UserDatabase::open(const QString &userId)
{
    const QString filePath(m_settings->databaseDir().filePath(userId + QLatin1String(".sqlite3")));
    return Database::open(filePath, userId + QLatin1String("-messenger"));
}

const AttachmentsTable *UserDatabase::attachmentsTable() const
{
    return static_cast<const AttachmentsTable *>(table(m_attachmentsTableIndex));
}

AttachmentsTable *UserDatabase::attachmentsTable()
{
    return static_cast<AttachmentsTable *>(table(m_attachmentsTableIndex));
}

const ChatsTable *UserDatabase::chatsTable() const
{
    return static_cast<const ChatsTable *>(table(m_chatsTableIndex));
}

ChatsTable *UserDatabase::chatsTable()
{
    return static_cast<ChatsTable *>(table(m_chatsTableIndex));
}

const ContactsTable *UserDatabase::contactsTable() const
{
    return static_cast<const ContactsTable *>(table(m_contactsTableIndex));
}

ContactsTable *UserDatabase::contactsTable()
{
    return static_cast<ContactsTable *>(table(m_contactsTableIndex));
}

const MessagesTable *UserDatabase::messagesTable() const
{
    return static_cast<const MessagesTable *>(table(m_messagesTableIndex));
}

MessagesTable *UserDatabase::messagesTable()
{
    return static_cast<MessagesTable *>(table(m_messagesTableIndex));
}

bool UserDatabase::create()
{
    tables().clear();
    int counter = -1;
    if (!addTable(std::make_unique<ChatsTable>("attachments_" + m_userId))) {
        return false;
    }
    m_attachmentsTableIndex = ++counter;
    if (!addTable(std::make_unique<ChatsTable>("chats_" + m_userId))) {
        return false;
    }
    m_chatsTableIndex = ++counter;
    if (!addTable(std::make_unique<ChatsTable>("contacts_" + m_userId))) {
        return false;
    }
    m_contactsTableIndex = ++counter;
    if (!addTable(std::make_unique<MessagesTable>("messages_" + m_userId))) {
        return false;
    }
    m_messagesTableIndex = ++counter;
    return true;
}
