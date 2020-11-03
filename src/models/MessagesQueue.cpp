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

#include "models/MessagesQueue.h"

#include "qxmpp/QXmppMessage.h"

#include "VSQMessenger.h"
#include "Utils.h"
#include "database/MessagesTable.h"
#include "database/UserDatabase.h"
#include "models/AttachmentsQueue.h"
#include "models/MessageOperation.h"
#include "models/Operation.h"
#include "models/SendMessageOperation.h"

using namespace vm;

MessagesQueue::MessagesQueue(VSQMessenger *messenger, UserDatabase *userDatabase, AttachmentsQueue *attachmentsQueue, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
    , m_userDatabase(userDatabase)
    , m_attachmentsQueue(attachmentsQueue)
    , m_root(new Operation("Queue", this))
{
    m_root->setInfinite();
    attachmentsQueue->setParent(this);
    connect(userDatabase, &UserDatabase::userIdChanged, this, &MessagesQueue::setUserId);
    //
    connect(this, &MessagesQueue::pushMessage, this, &MessagesQueue::onPushMessage);
    connect(this, &MessagesQueue::sendNotSentMessages, this, &MessagesQueue::onSendNotSentMessages);
}

MessagesQueue::~MessagesQueue()
{
}

void MessagesQueue::setUserId(const UserId &userId)
{
    if (m_userId == userId) {
        return;
    }

    m_root->dropChildren();
    m_userId = userId;
    if (!userId.isEmpty()) {
        connect(m_userDatabase->messagesTable(), &MessagesTable::notSentMessagesFetched, this, &MessagesQueue::setMessages);
        sendNotSentMessages();
    }
}

void MessagesQueue::setMessages(const GlobalMessages &messages)
{
    for (auto &m : messages) {
        createAppendOperation(m);
    }
    if (m_root->hasChildren()) {
        m_root->start();
    }
}

bool MessagesQueue::createAppendOperation(const GlobalMessage &message)
{
    auto op = new MessageOperation(message, this);
    // Append attachment operations
    m_attachmentsQueue->appendOperations(op);
    // Append send message operation
    if (message.senderId == m_userId) {
        if (message.status == Message::Status::Created || message.status == Message::Status::Failed) {
            op->appendChild(new SendMessageOperation(op, m_messenger->xmpp(), m_messenger->xmppURL()));
        }
    }
    // Check for empty
    if (!op->hasChildren()) {
        return false;
    }
    // Setup connections
    connect(op, &MessageOperation::statusChanged, this, std::bind(&MessagesQueue::onMessageOperationStatusChanged, this, op));
    // Append to root
    m_root->appendChild(op);
    return true;
}

void MessagesQueue::onPushMessage(const GlobalMessage &message)
{
    if (createAppendOperation(message)) {
        m_root->start();
    }
}

void MessagesQueue::onSendNotSentMessages()
{
    if (!m_userId.isEmpty()) {
        m_userDatabase->messagesTable()->fetchNotSentMessages();
    }
}

void MessagesQueue::onMessageOperationStatusChanged(const MessageOperation *operation)
{
    const auto m = operation->message();
    emit messageStatusChanged(m->id, m->contactId, m->status);
}
