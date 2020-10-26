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

#include "VSQMessenger.h"
#include "database/ChatsTable.h"
#include "database/UserDatabase.h"

using namespace vm;

ChatsController::ChatsController(VSQMessenger *messenger, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
    , m_userDatabase(userDatabase)
{
    connect(userDatabase, &UserDatabase::opened, this, &ChatsController::setupTableConnections);

    connect(m_messenger, &VSQMessenger::contactAdded, this, &ChatsController::chatCreated);
    connect(m_messenger, &VSQMessenger::addContactErrorOccured, this, &ChatsController::createChatErrorOccured);
}

void ChatsController::fetchChats()
{
    m_userDatabase->chatsTable()->fetch();
}

void ChatsController::createChat(const Contact::Id &contactId)
{
    m_messenger->addContact(contactId);
}

void ChatsController::resetUnreadCount(const Contact::Id &contactId)
{
    m_userDatabase->chatsTable()->resetUnreadCount(contactId);
}

void ChatsController::setupTableConnections()
{
    auto table = m_userDatabase->chatsTable();
    connect(table, &ChatsTable::fetched, this, &ChatsController::chatsFetched);
    connect(table, &ChatsTable::fetchErrorOccurred, this, &ChatsController::chatsFetchErrorOccurred);
    connect(m_messenger, &VSQMessenger::chatEntryRequested, table, &ChatsTable::createChat);
    connect(table, &ChatsTable::unreadCountReset, this, &ChatsController::unreadCountReset);
    connect(table, &ChatsTable::resetUnreadCountErrorOccurred, this, &ChatsController::resetUnreadCountErrorOccurred);
}
