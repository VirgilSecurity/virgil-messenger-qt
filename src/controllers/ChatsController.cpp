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

#include "Core.h"
#include "database/ChatsTable.h"
#include "database/MessagesTable.h"
#include "database/UserDatabase.h"
#include "models/ChatsModel.h"
#include "models/MessagesModel.h"
#include "models/Models.h"

using namespace vm;

ChatsController::ChatsController(Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_models(models)
    , m_userDatabase(userDatabase)
{
    connect(userDatabase, &UserDatabase::opened, this, &ChatsController::setupTableConnections);
    connect(this, &ChatsController::newContactFound, this, &ChatsController::onNewContactFound);
    connect(this, &ChatsController::groupChatCreated, this, &ChatsController::onGroupChatCreated);
    connect(this, &ChatsController::currentContactIdChanged, m_models->messages(), &MessagesModel::setContactId);
    connect(m_models->chats(), &ChatsModel::chatsSet, this, &ChatsController::onChatsSet);
}

Chat ChatsController::currentChat() const
{
    return m_currentChat;
}

Contact::Id ChatsController::currentContactId() const
{
    return m_currentChat.contactId;
}

Chat::Id ChatsController::currentChatId() const
{
    return m_currentChat.id;
}

void ChatsController::loadChats(const UserId &userId)
{
    qCDebug(lcController) << "Started to load chats...";
    m_userId = userId;
    if (userId.isEmpty()) {
        m_models->chats()->setChats({});
    }
    else {
        m_userDatabase->chatsTable()->fetch();
    }
}

void ChatsController::createChat(const Contact::Id &contactId)
{
    auto chat = m_models->chats()->findByContact(contactId);
    if (chat) {
        openChat(*chat);
    }
    else {
        // TODO(fpohtmeh): check if online?
        QtConcurrent::run([=]() {
            if (!Core::findContact(contactId)) {
                emit errorOccurred(tr("Contact not found"));
            }
            else {
                emit newContactFound(contactId, QPrivateSignal());
            }
        });
    }
}

void ChatsController::createGroupChat(const GroupId &groupId)
{
    auto chat = m_models->chats()->findById(groupId);
    if (chat) {
        openChat(*chat);
    }
    else {
        // TODO(fpohtmeh): check if online?
        QtConcurrent::run([=]() {
            emit groupChatCreated(groupId, QPrivateSignal());
        });
    }
}

void ChatsController::openChat(const Chat &chat)
{
    qCDebug(lcController) << "Opening chat with" << chat.contactId;
    setCurrentChat(chat);
    if (chat.unreadMessageCount > 0) {
        m_models->chats()->resetUnreadCount(chat.id);
        m_models->messages()->markAllAsRead();
        m_userDatabase->resetUnreadCount(chat);
    }
}

void ChatsController::openChatById(const Chat::Id &chatId)
{
    openChat(*m_models->chats()->findById(chatId));
}

void ChatsController::closeChat()
{
    setCurrentChat({});
}

void ChatsController::addParticipant(const UserId &userId)
{
    // addParticipant
}

void ChatsController::removeParticipant(const UserId &userId)
{
    // removeParticipant
}

void ChatsController::leaveGroup()
{
    auto currChatId = ChatsController::currentChatId();
    // leaveGroup(currChatId)
}

void ChatsController::setupTableConnections()
{
    auto table = m_userDatabase->chatsTable();
    connect(table, &ChatsTable::errorOccurred, this, &ChatsController::errorOccurred);
    connect(table, &ChatsTable::fetched, m_models->chats(), &ChatsModel::setChats);
}

void ChatsController::setCurrentChat(const Chat &chat)
{
    if (m_currentChat.id == chat.id) {
        return;
    }
    m_currentChat = chat;

    emit currentChatIdChanged(chat.id);
    emit currentContactIdChanged(chat.contactId);
    emit currentChatChanged(chat);
    emit chat.id.isEmpty() ? chatClosed() : chatOpened(chat);
}

void ChatsController::onNewContactFound(const Contact::Id &contactId)
{
    const auto chat = m_models->chats()->createChat(contactId);
    m_userDatabase->chatsTable()->createChat(chat);
    openChat(chat);
}

void ChatsController::onGroupChatCreated(const GroupId &groupId)
{
    const auto chat = m_models->chats()->createChat(groupId);
    m_userDatabase->chatsTable()->createChat(chat);
    openChat(chat);
}

void ChatsController::onChatsSet()
{
    qCDebug(lcController) << "Chats set";
    emit chatsSet(m_userId);
}
