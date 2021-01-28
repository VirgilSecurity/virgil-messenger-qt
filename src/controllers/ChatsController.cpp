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

#include <QtConcurrent>

#include "Messenger.h"
#include "database/ChatsTable.h"
#include "database/MessagesTable.h"
#include "database/UserDatabase.h"
#include "database/GroupMembersTable.h"
#include "database/GroupsTable.h"
#include "models/ChatsModel.h"
#include "models/MessagesModel.h"
#include "models/Models.h"
#include "Controller.h"
#include "Utils.h"

using namespace vm;
using Self = ChatsController;

Self::ChatsController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
    , m_models(models)
    , m_userDatabase(userDatabase)
{
    connect(userDatabase, &UserDatabase::opened, this, &Self::setupTableConnections);
    connect(this, &Self::createChatWithUser, this, &Self::onCreateChatWithUser);
    connect(this, &Self::createChatWithGroup, this, &Self::onCreateChatWithGroup);

    connect(m_messenger, &Messenger::groupChatCreated, this, &Self::onGroupChatCreated);
    connect(m_messenger, &Messenger::groupChatCreateFailed, this, &Self::onGroupChatCreateFailed);
    connect(m_messenger, &Messenger::updateGroup, this, &Self::onUpdateGroup);
}

ChatHandler Self::currentChat() const
{
    return m_currentChat;
}

void ChatsController::addParticipant(const QString &username) {
    Q_UNUSED(username)
}

void ChatsController::removeParticipant(const QString &username) {
    Q_UNUSED(username)
}

void ChatsController::leaveGroup() {

}

QString Self::currentChatName() const
{
    return m_currentChat ? m_currentChat->title() : QString();
}

void Self::loadChats()
{
    qCDebug(lcController) << "Started to load chats...";
    m_userDatabase->chatsTable()->fetch();
    m_userDatabase->groupMembersTable()->fetchByMemberId(m_messenger->currentUser()->id());
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
        }
        else {
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
        }
        else {
            emit errorOccurred(tr("Contact not found"));
        }
    });
}

void ChatsController::createGroupChat(const QString &groupName, const Contacts &contacts)
{
    QtConcurrent::run([this, groupName, contacts]() {
        if (!m_messenger) {
            qCWarning(lcController) << "Messenger was not initialized";
            emit errorOccurred(tr("Group con not be created"));
        }

        auto group = std::make_shared<Group>(GroupId::generate(), std::move(groupName), std::move(contacts));

        emit createChatWithGroup(group, QPrivateSignal());

        m_messenger->createGroupChat(group);
    });
}

void Self::openChat(const ChatHandler& chat)
{
    qCDebug(lcController) << "Opening chat with id: " << chat->id();
    if (chat->unreadMessageCount() > 0) {
        m_models->chats()->resetUnreadCount(chat->id());
        m_userDatabase->resetUnreadCount(chat);
    }
    m_currentChat = chat;
    emit chatOpened(m_currentChat);
    emit currentChatNameChanged(currentChatName());
}

void Self::openChat(const QString &chatId)
{
    openChat(m_models->chats()->findChat(ChatId(chatId)));
}

void Self::closeChat()
{
    m_currentChat = nullptr;
    emit chatClosed();
}

void Self::setupTableConnections()
{
    qCDebug(lcController) << "Setup database table connections for chats...";
    connect(m_userDatabase->chatsTable(), &ChatsTable::errorOccurred, this, &Self::errorOccurred);
    connect(m_userDatabase->chatsTable(), &ChatsTable::fetched, this, &Self::onChatsLoaded);

    connect(m_userDatabase->groupsTable(), &GroupsTable::errorOccurred, this, &Self::errorOccurred);
    connect(m_userDatabase->groupMembersTable(), &GroupMembersTable::errorOccurred, this, &Self::errorOccurred);
    connect(m_userDatabase->groupMembersTable(), &GroupMembersTable::fetched, this, &Self::onGroupMembersFetched);
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
    openChat(newChat);
}

void ChatsController::onCreateChatWithGroup(const GroupHandler &group)
{
    auto newChat = std::make_shared<Chat>();
    newChat->setId(ChatId(group->id()));
    newChat->setCreatedAt(QDateTime::currentDateTime());
    newChat->setType(Chat::Type::Group);
    newChat->setTitle(group->name());
    m_models->chats()->addChat(newChat);
    m_userDatabase->chatsTable()->addChat(newChat);
    openChat(newChat);
}

void Self::onChatsLoaded(ModifiableChats chats)
{
    qCDebug(lcController) << "Chats loaded from the database";
    m_models->chats()->setChats(std::move(chats));
    emit chatsLoaded();
}

void Self::onGroupChatCreated(const GroupId& groupId)
{
    auto createdChat = m_models->chats()->findChat(ChatId(QString(groupId)));
    if (createdChat) {
        emit chatCreated(createdChat);
    }
}


void Self::onGroupChatCreateFailed(const GroupId& chatId, const QString& errorText)
{
    //
    //  FIXME: Remove chat and show error text.
    //
}


void Self::onUpdateGroup(const GroupUpdate& groupUpdate)
{
    auto groupId = GroupUpdateGetId(groupUpdate);

    if (m_currentChat && m_currentChat->id() == groupId) {
        //
        //  FIXME: Update group info within chat.
        //
        //m_currentChat->updateGroup(groupUpdate);
    }

    m_userDatabase->updateGroup(groupUpdate);
}


void Self::onGroupMembersFetched(const GroupMembers& groupMembers)
{
    qCDebug(lcController) << "Group chats members with a current user was fetched";

    //
    //  When chats are loaded we need to join groups chats to be able receive messages.
    //
    m_messenger->joinGroupChats(groupMembers);

    //
    //  Disconnect to avoid calling this slot, when fetch group members within specific group.
    //
    disconnect(m_userDatabase->groupMembersTable(), &GroupMembersTable::fetched, this, &Self::onGroupMembersFetched);
}
