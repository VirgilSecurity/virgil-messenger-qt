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

#ifndef VM_USERDATABASE_H
#define VM_USERDATABASE_H

#include "core/Database.h"

#include <QDir>

namespace vm
{
class AttachmentsTable;
class ChatsTable;
class ContactsTable;
class MessagesTable;

class UserDatabase : public Database
{
    Q_OBJECT

public:
    explicit UserDatabase(const QDir &databaseDir, QObject *parent);
    ~UserDatabase() override;

    const AttachmentsTable *attachmentsTable() const;
    AttachmentsTable *attachmentsTable();
    const ChatsTable *chatsTable() const;
    ChatsTable *chatsTable();
    const ContactsTable *contactsTable() const;
    ContactsTable *contactsTable();
    const MessagesTable *messagesTable() const;
    MessagesTable *messagesTable();

signals:
    void requestOpen(const QString &username);
    void requestClose();

    void writeMessage(const Message &message);

    void usernameChanged(const QString &username);

private:
    bool create() override;
    void openByUsername(const QString &username);
    void close();

    void processWriteMessage(const Message &message);

    const QDir m_databaseDir;
    int m_attachmentsTableIndex;
    int m_chatsTableIndex;
    int m_contactsTableIndex;
    int m_messagesTableIndex;
};
}

#endif // VM_USERDATABASE_H
