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

#include "states/ChatState.h"

#include "Messenger.h"
#include "controllers/AttachmentsController.h"
#include "controllers/ChatsController.h"
#include "controllers/MessagesController.h"
#include "controllers/Controllers.h"
#include "MessageUpdate.h"

using namespace vm;

ChatState::ChatState(Controllers *controllers, Messenger *messenger, QState *parent)
    : QState(parent)
    , m_controllers(controllers)
{
    connect(m_controllers->attachments(), &AttachmentsController::openPreviewRequested, this, &ChatState::requestPreview);
    connect(messenger, &Messenger::lastActivityTextChanged, this, &ChatState::setLastActivityText);
    connect(messenger, &Messenger::messageSent, this, &ChatState::onMessageSent);
    setIsAdmin(true);
    setIsGroupChat(false);
}

QString ChatState::lastActivityText() const
{
    return m_lastActivityText;
}


bool ChatState::isAdmin() const
{
    return m_isAdmin;
}

bool ChatState::isGroupChat() const
{
    return m_isGroupChat;
}

void ChatState::setLastActivityText(const QString &text)
{
    if (text == m_lastActivityText) {
        return;
    }
    m_lastActivityText = text;
    emit lastActivityTextChanged(text);
}

void ChatState::setIsAdmin(const bool isAdmin)
{
    if (isAdmin == m_isAdmin) {
        return;
    }
    m_isAdmin = isAdmin;
    emit isAdminChanged(isAdmin);
}

void ChatState::setIsGroupChat(const bool isGroupChat)
{
    if (isGroupChat == m_isGroupChat) {
        return;
    }
    m_isGroupChat = isGroupChat;
    emit isGroupChatChanged(isGroupChat);
}

void ChatState::onMessageSent(MessageHandler message)
{
    const auto currentChat = m_controllers->chats()->currentChat();
    const auto currentChatId = currentChat ? currentChat->id() : QString();
    if (currentChatId == message->chatId()) {
        emit messageSent();
    }
}
