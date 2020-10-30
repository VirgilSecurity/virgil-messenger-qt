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

#include "controllers/Controllers.h"

#include "VSQMessenger.h"
#include "controllers/AttachmentsController.h"
#include "controllers/ChatsController.h"
#include "controllers/MessagesController.h"
#include "controllers/UsersController.h"
#include "database/UserDatabase.h"

using namespace vm;

Controllers::Controllers(VSQMessenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_attachments(new AttachmentsController(models, this))
    , m_users(new UsersController(messenger, models, userDatabase, this))
    , m_chats(new ChatsController(models, userDatabase, this))
    , m_messages(new MessagesController(messenger, models, userDatabase, this))
{
    connect(m_users, &UsersController::usernameChanged, m_chats, &ChatsController::loadChats);
    connect(m_chats, &ChatsController::currentContactIdChanged, messenger, &VSQMessenger::setCurrentRecipient); // TODO(fpohtmeh): remove connection
    connect(m_chats, &ChatsController::currentContactIdChanged, m_messages, &MessagesController::setRecipientId);
    connect(m_chats, &ChatsController::currentChatIdChanged, m_messages, &MessagesController::loadMessages);

    qRegisterMetaType<AttachmentsController *>("AttachmentsController*");
    qRegisterMetaType<ChatsController *>("ChatsController*");
    qRegisterMetaType<MessagesController *>("MessagesController*");
    qRegisterMetaType<UsersController *>("UsersController*");
}

const AttachmentsController *Controllers::attachments() const
{
    return m_attachments;
}

AttachmentsController *Controllers::attachments()
{
    return m_attachments;
}

const UsersController *Controllers::users() const
{
    return m_users;
}

UsersController *Controllers::users()
{
    return m_users;
}

const ChatsController *Controllers::chats() const
{
    return m_chats;
}

ChatsController *Controllers::chats()
{
    return m_chats;
}

const MessagesController *Controllers::messages() const
{
    return m_messages;
}

MessagesController *Controllers::messages()
{
    return m_messages;
}
