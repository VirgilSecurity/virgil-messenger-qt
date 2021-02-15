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
#include "Message.h"
#include "Chat.h"
#include "GroupUpdate.h"

#include <QDir>

namespace vm
{
class AttachmentsTable;
class ChatsTable;
class CloudFilesTable;
class ContactsTable;
class GroupMembersTable;
class GroupsTable;
class MessagesTable;

class UserDatabase : public Database
{
    Q_OBJECT

public:
    explicit UserDatabase(const QDir &databaseDir, QObject *parent);

    const AttachmentsTable *attachmentsTable() const;
    AttachmentsTable *attachmentsTable();

    const ChatsTable *chatsTable() const;
    ChatsTable *chatsTable();

    const CloudFilesTable *cloudFilesTable() const;
    CloudFilesTable *cloudFilesTable();

    const ContactsTable *contactsTable() const;
    ContactsTable *contactsTable();

    const GroupMembersTable *groupMembersTable() const;
    GroupMembersTable *groupMembersTable();

    const GroupsTable *groupsTable() const;
    GroupsTable *groupsTable();

    const MessagesTable *messagesTable() const;
    MessagesTable *messagesTable();

signals:
    //
    //  Control signals.
    //
    void open(const QString &username);
    void close();

    void writeMessage(const MessageHandler &message, qsizetype unreadCount = 0);
    void updateMessage(const MessageUpdate &messageUpdate);
    void writeChatAndLastMessage(const ChatHandler &chat);
    void resetUnreadCount(const ChatHandler &chat);
    void deleteNewGroupChat(const ChatId &chatId);
    void deleteGroupChatInvitation(const ChatId &chatId);

    void updateGroup(const GroupUpdate& groupUpdate);


    //
    //  Notification signals.
    //
    void opened();

private:
    bool create() override;
    void onOpen(const QString &username);
    void onClose();

    void onWriteMessage(const MessageHandler &message, qsizetype unreadCount);
    void onUpdateMessage(const MessageUpdate &messageUpdate);
    void onWriteChatAndLastMessage(const ChatHandler &chat);
    void onResetUnreadCount(const ChatHandler &chat);
    void onDeleteNewGroupChat(const ChatId &chatId);
    void onDeleteGroupChatInvitation(const ChatId &chatId);

    void onUpdateGroup(const GroupUpdate& groupUpdate);

    const QDir m_databaseDir;
};
}

#endif // VM_USERDATABASE_H
