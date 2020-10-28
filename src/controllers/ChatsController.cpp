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
#include "database/UserDatabase.h"
#include "models/ChatsModel.h"
#include "models/Models.h"

using namespace vm;

ChatsController::ChatsController(Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_models(models)
    , m_userDatabase(userDatabase)
{
    connect(userDatabase, &UserDatabase::opened, this, &ChatsController::setupTableConnections);
    connect(this, &ChatsController::contactFound, this, &ChatsController::onContactFound);
}

Contact::Id ChatsController::currentContactId() const
{
    return m_currentContactId;
}

void ChatsController::loadChats(const QString &username)
{
    if (username.isEmpty()) {
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
                emit contactFound(contactId, QPrivateSignal());
            }
        });
    }
}

void ChatsController::openChat(const Chat &chat)
{
    if (chat.unreadMessageCount > 0) {
        m_models->chats()->resetUnreadCount(chat.id);
        m_userDatabase->chatsTable()->resetUnreadCount(chat.id);
    }
    setCurrentChatId(chat.id);
    setCurrentContactId(chat.contactId);
    emit chatOpened(chat.id);
}

void ChatsController::openChatById(const Chat::Id &chatId)
{
    openChat(*m_models->chats()->find(chatId));
}

void ChatsController::closeChat()
{
    if (!m_currentContactId.isEmpty()) {
        setCurrentChatId(QString());
        setCurrentContactId(QString());
        emit chatClosed();
    }
}

void ChatsController::setupTableConnections()
{
    auto table = m_userDatabase->chatsTable();
    connect(table, &ChatsTable::errorOccurred, this, &ChatsController::errorOccurred);
    connect(table, &ChatsTable::fetched, m_models->chats(), &ChatsModel::setChats);
}

void ChatsController::setCurrentContactId(const Contact::Id &contactId)
{
    if (m_currentContactId == contactId) {
        return;
    }
    m_currentContactId = contactId;
    emit currentContactIdChanged(contactId);
}

void ChatsController::setCurrentChatId(const Chat::Id &chatId)
{
    if (m_currentChatId == chatId) {
        return;
    }
    m_currentChatId = chatId;
    emit currentChatIdChanged(chatId);
}

void ChatsController::onContactFound(const Contact::Id &contactId)
{
    const auto chat = m_models->chats()->createChat(contactId);
    m_userDatabase->chatsTable()->createChat(chat);
    setCurrentChatId(chat.id);
    setCurrentContactId(contactId);
    emit chatOpened(chat.id);
}
