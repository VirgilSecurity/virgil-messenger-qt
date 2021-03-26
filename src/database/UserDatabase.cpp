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

#include "Settings.h"
#include "Utils.h"
#include "database/core/DatabaseUtils.h"
#include "database/AttachmentsTable.h"
#include "database/ChatsTable.h"
#include "database/CloudFilesTable.h"
#include "database/ContactsTable.h"
#include "database/GroupMembersTable.h"
#include "database/GroupsTable.h"
#include "database/MessagesTable.h"
#include "database/UserDatabaseMigration.h"

#include "MessageContentGroupInvitation.h"

using namespace vm;
using Self = UserDatabase;

constexpr const int k_attachmentsTableIndex = 0;
constexpr const int k_chatsTableIndex = 1;
constexpr const int k_contactsTableIndex = 2;
constexpr const int k_cloudFilesTableIndex = 3;
constexpr const int k_groupMembersTableIndex = 4;
constexpr const int k_groupsTableIndex = 5;
constexpr const int k_messagesTableIndex = 6;

Self::UserDatabase(const QDir &databaseDir, QObject *parent)
    : Database(VERSION_DATABASE_SCHEME, parent), m_databaseDir(databaseDir)
{
    setMigration(std::make_unique<UserDatabaseMigration>());

    connect(this, &Self::open, this, &Self::onOpen);
    connect(this, &Self::close, this, &Self::onClose);
    connect(this, &Self::writeMessage, this, &Self::onWriteMessage);
    connect(this, &Self::updateMessage, this, &Self::onUpdateMessage);
    connect(this, &Self::writeChatAndLastMessage, this, &Self::onWriteChatAndLastMessage);
    connect(this, &Self::writeGroupChat, this, &Self::onWriteGroupChat);
    connect(this, &Self::deleteNewGroupChat, this, &Self::onDeleteNewGroupChat);
    connect(this, &Self::updateGroup, this, &Self::onUpdateGroup);
}

const AttachmentsTable *Self::attachmentsTable() const
{
    return static_cast<const AttachmentsTable *>(table(k_attachmentsTableIndex));
}

AttachmentsTable *Self::attachmentsTable()
{
    return static_cast<AttachmentsTable *>(table(k_attachmentsTableIndex));
}

const ChatsTable *Self::chatsTable() const
{
    return static_cast<const ChatsTable *>(table(k_chatsTableIndex));
}

ChatsTable *Self::chatsTable()
{
    return static_cast<ChatsTable *>(table(k_chatsTableIndex));
}

const CloudFilesTable *UserDatabase::cloudFilesTable() const
{
    return static_cast<const CloudFilesTable *>(table(k_cloudFilesTableIndex));
}

CloudFilesTable *UserDatabase::cloudFilesTable()
{
    return static_cast<CloudFilesTable *>(table(k_cloudFilesTableIndex));
}

const ContactsTable *Self::contactsTable() const
{
    return static_cast<const ContactsTable *>(table(k_contactsTableIndex));
}

ContactsTable *Self::contactsTable()
{
    return static_cast<ContactsTable *>(table(k_contactsTableIndex));
}

const GroupMembersTable *Self::groupMembersTable() const
{
    return static_cast<const GroupMembersTable *>(table(k_groupMembersTableIndex));
}

GroupMembersTable *Self::groupMembersTable()
{
    return static_cast<GroupMembersTable *>(table(k_groupMembersTableIndex));
}

const GroupsTable *Self::groupsTable() const
{
    return static_cast<const GroupsTable *>(table(k_groupsTableIndex));
}

GroupsTable *Self::groupsTable()
{
    return static_cast<GroupsTable *>(table(k_groupsTableIndex));
}

const MessagesTable *Self::messagesTable() const
{
    return static_cast<const MessagesTable *>(table(k_messagesTableIndex));
}

MessagesTable *Self::messagesTable()
{
    return static_cast<MessagesTable *>(table(k_messagesTableIndex));
}

bool Self::create()
{
    tables().clear();

    Q_ASSERT(k_attachmentsTableIndex == tables().size());
    if (!addTable(std::make_unique<AttachmentsTable>(this))) {
        return false;
    }

    Q_ASSERT(k_chatsTableIndex == tables().size());
    if (!addTable(std::make_unique<ChatsTable>(this))) {
        return false;
    }

    Q_ASSERT(k_contactsTableIndex == tables().size());
    if (!addTable(std::make_unique<ContactsTable>(this))) {
        return false;
    }

    Q_ASSERT(k_cloudFilesTableIndex == tables().size());
    if (!addTable(std::make_unique<CloudFilesTable>(this))) {
        return false;
    }

    Q_ASSERT(k_groupMembersTableIndex == tables().size());
    if (!addTable(std::make_unique<GroupMembersTable>(this))) {
        return false;
    }

    Q_ASSERT(k_groupsTableIndex == tables().size());
    if (!addTable(std::make_unique<GroupsTable>(this))) {
        return false;
    }

    Q_ASSERT(k_messagesTableIndex == tables().size());
    if (!addTable(std::make_unique<MessagesTable>(this))) {
        return false;
    }

    connect(messagesTable(), &MessagesTable::chatUnreadMessageCountChanged, chatsTable(),
            &ChatsTable::requestChatUnreadMessageCount);

    return true;
}

void Self::onOpen(const QString &username)
{
    if (!DatabaseUtils::isValidName(username)) {
        qCCritical(lcDatabase) << "Invalid database id:" << username;
        emit errorOccurred(tr("Invalid database id"));
    } else {
        const QString fileName = QString("user-%1.sqlite3").arg(username);
        const QString filePath(m_databaseDir.filePath(fileName));

        if (Database::open(filePath, username + QLatin1String("-messenger"))) {
            emit opened();
        } else {
            emit errorOccurred(tr("Can not open database with users"));
        }
    }
}

void Self::onClose()
{
    Database::close();
}

void Self::onWriteMessage(const MessageHandler &message)
{
    ScopedConnection connection(*this);
    ScopedTransaction transaction(*this);
    messagesTable()->addMessage(message);

    if (rowsChangedCount() > 0) {
        if (message->contentIsAttachment()) {
            attachmentsTable()->addAttachment(message);
        }

        if (message->isIncoming()) {
            contactsTable()->updateContact(UsernameContactUpdate { message->senderId(), message->senderUsername() });
        }

        chatsTable()->updateLastMessage(message);
    }
}

void UserDatabase::onUpdateMessage(const MessageUpdate &messageUpdate)
{
    // Early ignore of updates that doesn't write to DB
    if (std::holds_alternative<MessageAttachmentProcessedSizeUpdate>(messageUpdate)) {
        return;
    }

    {
        ScopedConnection connection(*this);
        ScopedTransaction transaction(*this);
        messagesTable()->updateMessage(messageUpdate);
        attachmentsTable()->updateAttachment(messageUpdate);
    }
}

void Self::onWriteChatAndLastMessage(const ChatHandler &chat)
{
    ScopedConnection connection(*this);
    ScopedTransaction transaction(*this);
    //
    // Create chat without last message.
    //
    chatsTable()->addChat(chat);

    //
    // Create groups for group chat.
    //
    if (chat->type() == ChatType::Group) {

        //
        //  Expect invitation message here.
        //
        const auto message = chat->lastMessage();

        if (const auto content = std::get_if<MessageContentGroupInvitation>(&message->content())) {

            const auto groupId = GroupId(chat->id());

            auto group = std::make_shared<Group>(groupId, content->superOwnerId(), content->title(),
                                                 GroupInvitationStatus::Invited);

            groupsTable()->add(group);

            if (rowsChangedCount() > 0) {
                groupMembersTable()->updateGroup(
                        GroupMemberAffiliationUpdate { groupId, content->superOwnerId(), GroupAffiliation::Owner });
            }
        }
    }

    //
    // Create message.
    //
    const auto message = chat->lastMessage();
    messagesTable()->addMessage(message);

    //
    // Create attachment (optional).
    //
    if (message->contentIsAttachment()) {
        attachmentsTable()->addAttachment(message);
    }

    // Update last message
    chatsTable()->updateLastMessage(message);

    if (message->isIncoming()) {
        contactsTable()->updateContact(UsernameContactUpdate { message->senderId(), message->senderUsername() });
    }

    chatsTable()->requestChatUnreadMessageCount(message->chatId());
}

void Self::onWriteGroupChat(const ChatHandler &chat, const GroupHandler &group, const GroupMembers &groupMembers)
{
    ScopedConnection connection(*this);
    ScopedTransaction transaction(*this);
    chatsTable()->addChat(chat);
    groupsTable()->add(group);
    if (!groupMembers.empty()) {
        groupMembersTable()->add(groupMembers);
    }
}

void Self::onDeleteNewGroupChat(const ChatId &chatId)
{
    ScopedConnection connection(*this);
    ScopedTransaction transaction(*this);
    // NOTE(fpohtmeh): new group chat has no attachments
    messagesTable()->deleteChatMessages(chatId);
    chatsTable()->deleteChat(chatId);
    const GroupId groupId(chatId);
    groupsTable()->deleteGroup(groupId);
    groupMembersTable()->deleteGroupMembers(groupId);
}

void Self::onUpdateGroup(const GroupUpdate &groupUpdate)
{
    ScopedConnection connection(*this);
    ScopedTransaction transaction(*this);
    groupsTable()->updateGroup(groupUpdate);
    groupMembersTable()->updateGroup(groupUpdate);
}
