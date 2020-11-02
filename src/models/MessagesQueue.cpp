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

#include "Core.h"
#include "VSQMessenger.h"
#include "Utils.h"
#include "database/MessagesTable.h"
#include "database/UserDatabase.h"

using namespace vm;

MessagesQueue::MessagesQueue(VSQMessenger *messenger, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
    , m_userDatabase(userDatabase)
{
    connect(userDatabase, &UserDatabase::userIdChanged, this, &MessagesQueue::setUserId);
    //
    connect(this, &MessagesQueue::pushMessage, this, &MessagesQueue::onPushMessage);
    connect(this, &MessagesQueue::sendFailedMessages, this, &MessagesQueue::onSendFailedMessages);
    connect(this, &MessagesQueue::sendNextMessage, this, &MessagesQueue::onSendNextMessage, Qt::QueuedConnection); // queue connection to avoid recursion
    connect(this, &MessagesQueue::messageStatusChanged, this, &MessagesQueue::onMessageStatusChanged);
}

void MessagesQueue::setUserId(const UserId &userId)
{
    if (m_userId == userId) {
        return;
    }
    m_userId = userId;
    //
    m_messages = {};
    if (!userId.isEmpty()) {
        connect(m_userDatabase->messagesTable(), &MessagesTable::failedMessagesFetched, this, &MessagesQueue::setFailedMessages);
        sendFailedMessages();
    }
}

void MessagesQueue::setFailedMessages(const QueueMessages &messages)
{
    if (messages.empty()) {
        return;
    }
    for (auto &m : messages) {
        m_messages.push(m);
    }
    qDebug() << "Enqueued" << messages.size() << "failed messages";
    sendNextMessage(QPrivateSignal());
}

void MessagesQueue::onPushMessage(const Message &message, const Contact::Id &sender, const Contact::Id &recipient)
{
    m_messages.push({ message, sender, recipient });
    sendNextMessage(QPrivateSignal());
}

void MessagesQueue::onSendFailedMessages()
{
    if (!m_userId.isEmpty()) {
        m_userDatabase->messagesTable()->fetchFailedMessages();
    }
}

void MessagesQueue::onSendNextMessage()
{
    if(m_messages.empty()) {
        return;
    }

    auto &message = m_messages.front();
    bool processed = false;
    if (message.status != Message::Status::Created && message.status != Message::Status::Failed) {
        processed = true;
    }
    else if (message.senderId == m_userId) {
        if (!message.attachment) {
            const auto status = sendMessage(message);
            if (message.status != status) {
                message.status = status;
                emit messageStatusChanged(message.id, message.recipientId, status);
            }
            processed = true;
        }
    }
    else if (message.recipientId == m_userId) {
        if (!message.attachment) {
            processed = true;
        }
    }

    if (processed) {
        m_messages.pop();
    }
    else {
        qWarning() << "Message wasn't processed by queue. Id:" << message.id << "sender;" << message.senderId << "recipient:" << message.recipientId
                   << "body:" << Utils::printableMessageBody(message);
    }
}

void MessagesQueue::onMessageStatusChanged(const Message::Id &messageId, const Contact::Id &contactId, const Message::Status status)
{
    Q_UNUSED(messageId)
    Q_UNUSED(contactId)
    if (status == Message::Status::Sent) {
        sendNextMessage(QPrivateSignal());
    }
}

Message::Status MessagesQueue::sendMessage(const QueueMessage &message)
{
    qDebug() << "Message sending started:" << message.id;
    const auto encryptedStr = Core::encryptMessage(message, message.recipientId);
    if (!encryptedStr) {
        return Message::Status::InvalidM;
    }
    else {
        const auto fromJID = Utils::createJid(message.senderId, m_messenger->xmppURL());
        const auto toJID = Utils::createJid(message.recipientId, m_messenger->xmppURL());

        QXmppMessage msg(fromJID, toJID, *encryptedStr);
        msg.setReceiptRequested(true);
        msg.setId(message.id);

        if (m_messenger->xmpp()->sendPacket(msg)) {
            qDebug() << "Message sent:" << message.id;
            return Message::Status::Sent;
        } else {
            return Message::Status::Failed;
        }
    }
}
