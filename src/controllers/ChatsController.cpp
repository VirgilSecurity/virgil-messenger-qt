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
    connect(this, &ChatsController::chatOpened, models->chats(), &ChatsModel::addChat);
    connect(this, &ChatsController::chatOpened, this, &ChatsController::resetUnreadCount);
    connect(this, &ChatsController::chatOpened, this, &ChatsController::setChatContact);
    connect(this, &ChatsController::chatClosed, this, std::bind(&ChatsController::setChatContact, this, QString()));
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

void ChatsController::openChat(const Contact::Id &contactId)
{
    if (m_models->chats()->hasChat(contactId)) {
        emit chatOpened(contactId);
    }
    else {
        // TODO(fpohtmeh): check if online?
        QtConcurrent::run([=]() {
            if (!Core::findContact(contactId)) {
                emit errorOccurred(tr("Contact not found"));
            }
            else {
                emit chatOpened(contactId);
            }
        });
    }
}

void ChatsController::closeChat()
{
    if (!m_chatContact.isEmpty()) {
        emit chatClosed();
    }
}

void ChatsController::resetUnreadCount(const Contact::Id &contactId)
{
    m_models->chats()->resetUnreadCount(contactId);
    m_userDatabase->chatsTable()->resetUnreadCount(contactId);
}

void ChatsController::setupTableConnections()
{
    auto table = m_userDatabase->chatsTable();
    connect(table, &ChatsTable::errorOccurred, this, &ChatsController::errorOccurred);
    connect(table, &ChatsTable::fetched, m_models->chats(), &ChatsModel::setChats);
    connect(this, &ChatsController::chatOpened, table, &ChatsTable::createChat);
}

void ChatsController::setChatContact(const Contact::Id &contactId)
{
    if (m_chatContact == contactId) {
        return;
    }
    m_chatContact = contactId;
    emit chatContactChanged(contactId);
}
