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

#include "controllers/MessagesController.h"

#include "VSQMessenger.h"
#include "controllers/ChatsController.h"
#include "database/MessagesTable.h"
#include "database/UserDatabase.h"
#include "models/AttachmentsModel.h"
#include "models/ChatsModel.h"
#include "models/MessagesModel.h"
#include "models/Models.h"

using namespace vm;

MessagesController::MessagesController(VSQMessenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
    , m_models(models)
    , m_userDatabase(userDatabase)
{
    connect(userDatabase, &UserDatabase::opened, this, &MessagesController::setupTableConnections);
}

void MessagesController::loadMessages(const Contact::Id &chatId)
{
    m_chatId = chatId;
    if (chatId.isEmpty()) {
        m_models->messages()->setMessages({});
    }
    else {
        m_userDatabase->messagesTable()->fetch(chatId);
    }
}

void MessagesController::sendMessage(const QString &body, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType)
{
    if (body.isEmpty() && !attachmentUrl.isValid()) {
        qDebug() << "Text and attachment are empty";
        return;
    }
    const auto attachment = m_models->attachments()->createAttachment(attachmentUrl.toUrl(), attachmentType);
    auto message = m_models->messages()->createMessage(m_chatId, m_recipientId, body, attachment);
    m_models->chats()->updateLastMessage(message);
    m_userDatabase->writeMessage(message);
    // FIXME(fpohtmeh): implement
    //m_messenger->sendMessage(body, attachmentUrl, attachmentType);
}

void MessagesController::setRecipientId(const Contact::Id &recipientId)
{
    m_recipientId = recipientId;
}

void MessagesController::setupTableConnections()
{
    auto table = m_userDatabase->messagesTable();
    connect(table, &MessagesTable::errorOccurred, this, &MessagesController::errorOccurred);
    connect(table, &MessagesTable::fetched, m_models->messages(), &MessagesModel::setMessages);
}
