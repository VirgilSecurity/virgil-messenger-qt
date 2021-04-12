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

#include "controllers/ChatsController.h"

#include "Messenger.h"
#include "database/ChatsTable.h"
#include "database/MessagesTable.h"
#include "database/UserDatabase.h"
#include "database/GroupMembersTable.h"
#include "database/GroupsTable.h"
#include "database/ContactsTable.h"
#include "models/ChatsModel.h"
#include "models/MessagesModel.h"
#include "models/Models.h"
#include "MessageContentJsonUtils.h"
#include "Controller.h"
#include "Utils.h"

#include <QtConcurrent>

#include <algorithm>

using namespace vm;
using Self = ChatsController;

Self::ChatsController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent),
      m_messenger(messenger),
      m_models(models),
      m_userDatabase(userDatabase),
      m_chatObject(new ChatObject(messenger, this))
{
    connect(userDatabase, &UserDatabase::opened, this, &Self::setupTableConnections);

    connect(this, &Self::createChatWithUser, this, &Self::onCreateChatWithUser);

    connect(m_messenger, &Messenger::groupChatCreated, this, &Self::onGroupChatCreated);
    connect(m_messenger, &Messenger::groupChatCreateFailed, this, &Self::onGroupChatCreateFailed);
    connect(m_messenger, &Messenger::updateGroup, this, &Self::onUpdateGroup);
    connect(m_messenger, &Messenger::newGroupChatLoaded, this, &Self::onNewGroupChatLoaded);
}

ChatHandler Self::currentChat() const
{
    return m_chatObject->chat();
}

void Self::loadGroupMembers()
{
    if (currentChat()->type() == ChatType::Group) {
        const auto chatId(currentChat()->id());
        m_userDatabase->groupMembersTable()->fetch(GroupId(chatId));
    }
}

void Self::acceptGroupInvitation(const MessageHandler &invitationMessage)
{
    const GroupId groupId(invitationMessage->chatId());
    const auto invitation = std::get_if<MessageContentGroupInvitation>(&invitationMessage->content());
    m_messenger->acceptGroupInvitation(groupId, invitation->superOwnerId());
}

void Self::rejectGroupInvitation(const MessageHandler &invitationMessage)
{
    const auto chatId(invitationMessage->chatId());
    const auto invitation = std::get_if<MessageContentGroupInvitation>(&invitationMessage->content());
    m_messenger->rejectGroupInvitation(GroupId(chatId), invitation->superOwnerId());

    m_models->chats()->deleteChat(chatId);
    m_userDatabase->deleteNewGroupChat(chatId);
    emit groupInvitationRejected();
}

void Self::addMembers(const Contacts &contacts)
{
    const GroupId groupId(currentChat()->id());
    const auto groupMembers = ContactsToGroupMembers(groupId, contacts);
    qCWarning(lcController) << "ChatsController::addMembers is under development"
                            << Utils::printableContactsList(contacts);
}

void Self::removeSelectedMembers()
{
    const GroupId groupId(currentChat()->id());
    const auto groupMembers = m_chatObject->selectedGroupMembers();
    qCWarning(lcController) << "ChatsController::removeSelectedMembers is under development";
}

void Self::leaveGroup()
{
    const GroupId groupId(currentChat()->id());
    qCWarning(lcController) << "ChatsController::leaveGroup is under development";
}

void Self::loadChats()
{
    qCDebug(lcController) << "Started to load chats...";
    m_userDatabase->chatsTable()->fetch();
    m_userDatabase->groupsTable()->fetch();
}

void Self::clearChats()
{
    qCDebug(lcController) << "Clear chats...";
    m_models->chats()->clearChats();
}

void Self::createChatWithUsername(const QString &username)
{
    QtConcurrent::run([this, username = username]() {
        if (!m_messenger) {
            qCWarning(lcController) << "Messenger was not initialized";
            emit errorOccurred(tr("Chat can not be created"));
        }
        auto user = m_messenger->findUserByUsername(username);
        if (user) {
            emit createChatWithUser(user, QPrivateSignal());
        } else {
            emit errorOccurred(tr("Contact not found"));
        }
    });
}

void Self::createChatWithUserId(const UserId &userId)
{
    QtConcurrent::run([this, userId = userId]() {
        if (!m_messenger) {
            qCWarning(lcController) << "Messenger was not initialized";
            emit errorOccurred(tr("Contact not found"));
        }
        auto user = m_messenger->findUserById(userId);
        if (user) {
            emit createChatWithUser(user, QPrivateSignal());
        } else {
            emit errorOccurred(tr("Contact not found"));
        }
    });
}

void ChatsController::createGroupChat(const QString &groupName, const Contacts &contacts)
{
    if (!m_messenger) {
        qCWarning(lcController) << "Messenger was not initialized";
        emit errorOccurred(tr("Group can not be created"));
    }

    //
    //  This invocation runs concurrently.
    //
    m_messenger->createGroupChat(groupName, contacts);
}

void Self::openChat(const ModifiableChatHandler &chat, bool isNew)
{
    qCDebug(lcController) << "Opening chat with id: " << chat->id();
    if (!isNew && chat->unreadMessageCount() > 0) {
        m_userDatabase->chatsTable()->markMessagesAsRead(chat);
    }
    setCurrentChat(chat);
    emit chatOpened(chat, isNew);
}

void Self::openChat(const QString &chatId)
{
    openChat(m_models->chats()->findChat(ChatId(chatId)), false);
}

void Self::closeChat()
{
    setCurrentChat(ModifiableChatHandler());
    emit chatClosed();
}

void Self::setupTableConnections()
{
    qCDebug(lcController) << "Setup database table connections for chats...";
    connect(m_userDatabase->chatsTable(), &ChatsTable::errorOccurred, this, &Self::errorOccurred);
    connect(m_userDatabase->chatsTable(), &ChatsTable::fetched, this, &Self::onChatsLoaded);
    connect(m_userDatabase->messagesTable(), &MessagesTable::messageAdded, this, &Self::onMessageAdded);
    connect(m_userDatabase->chatsTable(), &ChatsTable::chatUnreadMessageCountUpdated, this,
            &Self::onChatUnreadMessageCountUpdated);
    connect(m_userDatabase->chatsTable(), &ChatsTable::lastUnreadMessageBeforeItWasRead, this,
            &Self::onLastUnreadMessageBeforeItWasRead);

    connect(m_userDatabase->groupsTable(), &GroupsTable::fetched, this, &Self::onGroupsFetched);
    connect(m_userDatabase->groupsTable(), &GroupsTable::added, this, &Self::onDatabaseGroupAdded);
    connect(m_userDatabase->groupsTable(), &GroupsTable::errorOccurred, this, &Self::errorOccurred);
    connect(m_userDatabase->groupMembersTable(), &GroupMembersTable::errorOccurred, this, &Self::errorOccurred);
    connect(m_userDatabase->groupMembersTable(), &GroupMembersTable::fetched, this, &Self::onGroupMembersFetched);

    // TODO: Move to an appropriate place.
    connect(m_messenger, &Messenger::updateContact, m_userDatabase->contactsTable(), &ContactsTable::updateContact);
}

void Self::setCurrentChat(ModifiableChatHandler chat)
{
    m_chatObject->setChat(chat);
    m_models->chats()->selectChatOnly(chat);
}

void Self::onCreateChatWithUser(const UserHandler &user)
{
    auto newChat = std::make_shared<Chat>();
    newChat->setId(ChatId(user->id()));
    newChat->setCreatedAt(QDateTime::currentDateTime());
    newChat->setType(Chat::Type::Personal);
    newChat->setTitle(user->username().isEmpty() ? user->id() : user->username());
    m_models->chats()->addChat(newChat);
    m_userDatabase->chatsTable()->addChat(newChat);
    openChat(newChat, true);
}

void Self::onChatsLoaded(ModifiableChats chats)
{
    qCDebug(lcController) << "Chats loaded from the database";
    m_models->chats()->setChats(std::move(chats));
    emit chatsLoaded();
}

void Self::onGroupChatCreated(const GroupHandler &group, const GroupMembers &groupMembers)
{
    auto newChat = std::make_shared<Chat>();
    newChat->setId(ChatId(group->id()));
    newChat->setCreatedAt(QDateTime::currentDateTime());
    newChat->setType(Chat::Type::Group);
    newChat->setGroup(group);
    newChat->setTitle(group->name());
    m_models->chats()->addChat(newChat);
    m_userDatabase->writeGroupChat(newChat, group, groupMembers);
    openChat(newChat, true);
}

void Self::onGroupChatCreateFailed(const GroupId &chatId, const QString &errorText)
{
    Q_UNUSED(chatId)
    emit notificationCreated(errorText, true);
}

void Self::onUpdateGroup(const GroupUpdate &groupUpdate)
{
    //
    //  Update UI.
    //
    m_chatObject->updateGroup(groupUpdate);
    m_models->chats()->updateGroup(groupUpdate);
    m_models->messages()->updateGroup(groupUpdate);

    //
    //  Update DB.
    //
    m_userDatabase->updateGroup(groupUpdate);
}

void Self::onNewGroupChatLoaded(const GroupHandler &group)
{
    auto newChat = std::make_shared<Chat>();
    newChat->setId(ChatId(group->id()));
    newChat->setCreatedAt(QDateTime::currentDateTime());
    newChat->setType(Chat::Type::Group);
    newChat->setGroup(group);
    newChat->setTitle(group->name());
    m_models->chats()->addChat(newChat);
    m_userDatabase->writeGroupChat(newChat, group, {});
}

void Self::onDatabaseGroupAdded(const GroupHandler &group)
{
    if (auto chat = m_models->chats()->findChat(ChatId(group->id()))) {
        chat->setGroup(group);
    }
}

void Self::onGroupsFetched(const Groups &groups)
{
    qCDebug(lcController) << "Groups were fetched";
    m_messenger->loadGroupChats(groups);
}

void Self::onGroupMembersFetched(const GroupId &groupId, const GroupMembers &groupMembers)
{
    if (currentChat()->id() == groupId) {
        qCDebug(lcController) << "Group chats members with a current group were fetched" << groupId;
        m_chatObject->setGroupMembers(groupMembers);
    }
}

void Self::onMessageAdded(const MessageHandler &message)
{
    if (message->isIncoming()) {
        m_userDatabase->chatsTable()->requestChatUnreadMessageCount(message->chatId());
    }
}

void Self::onChatUnreadMessageCountUpdated(const ChatId &chatId, qsizetype unreadMessageCount)
{
    m_models->chats()->resetUnreadCount(chatId, unreadMessageCount);
}

void Self::onLastUnreadMessageBeforeItWasRead(const MessageHandler &message)
{
    m_messenger->sendMessageStatusDisplayed(message);
}
