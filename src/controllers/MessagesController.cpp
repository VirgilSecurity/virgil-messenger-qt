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

#include <QtConcurrent>

#include <qxmpp/QXmppCarbonManager.h>
#include <qxmpp/QXmppClient.h>
#include <qxmpp/QXmppMessage.h>
#include <qxmpp/QXmppMessageReceiptManager.h>

#include "Core.h"
#include "VSQMessenger.h"
#include "Utils.h"
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
    connect(userDatabase, &UserDatabase::usernameChanged, this, &MessagesController::setUserId);
    connect(messenger, &VSQMessenger::messageStatusChanged, this, &MessagesController::setMessageStatus);
    connect(messenger, &VSQMessenger::fireReady, this, &MessagesController::sendNotSentMessages);
    //
    connect(this, &MessagesController::messageStatusChanged, this, &MessagesController::setMessageStatus);
    // Xmpp connections
    auto xmpp = messenger->xmpp();
    connect(xmpp, &QXmppClient::messageReceived, this, &MessagesController::receiveMessage);
    auto receipt = xmpp->findExtension<QXmppMessageReceiptManager>();
    connect(receipt, &QXmppMessageReceiptManager::messageDelivered, this, &MessagesController::setDeliveredStatus);
    auto carbon = xmpp->findExtension<QXmppCarbonManager>();
    connect(carbon, &QXmppCarbonManager::messageReceived, xmpp, &QXmppClient::messageReceived); // sent to our account (forwarded from another client)
    connect(carbon, &QXmppCarbonManager::messageSent, xmpp, &QXmppClient::messageReceived); // sent from our account (but another client)
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

void MessagesController::createSendMessage(const QString &body, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType)
{
    if (body.isEmpty() && !attachmentUrl.isValid()) {
        qDebug() << "Text and attachment are empty";
        return;
    }
    const auto attachment = m_models->attachments()->createAttachment(attachmentUrl.toUrl(), attachmentType);
    auto message = m_models->messages()->createMessage(m_chatId, m_recipientId, body, attachment);
    m_userDatabase->writeMessage(message);
    sendMessage(message);
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

void MessagesController::setUserId(const UserId &userId)
{
    m_userId = userId;
}

void MessagesController::setMessageStatus(const Message::Id &messageId, const Contact::Id &contactId, const Message::Status &status)
{
    qDebug() << "Set message status:" << messageId << contactId << status;
    if (contactId == m_recipientId) {
        if (!m_models->messages()->setMessageStatus(messageId, status)) {
            return;
        }
    }
    m_userDatabase->messagesTable()->updateStatus(messageId, status);
}

void MessagesController::setDeliveredStatus(const Jid &jid, const Message::Id &messageId)
{
    const auto contactId = Utils::contactIdFromJid(jid);
    qDebug() << "Message with id:" << messageId << "delivered to" << contactId;
    setMessageStatus(messageId, contactId, Message::Status::Delivered);
}

void MessagesController::receiveMessage(const QXmppMessage &msg)
{
    const auto senderId = Utils::contactIdFromJid(msg.from());
    const auto recipientId = Utils::contactIdFromJid(msg.to());
    qInfo() << "Received message from:" << senderId << "recipient:" << recipientId;
    // Decrypt message
    auto message = Core::decryptMessage(msg.id(), senderId, msg.body());
    // FIXME(fpohtmeh): add chatId, senderId, timestamp
    if (!message) {
        return;
    }

    if (senderId == m_userId) {
        message->status = Message::Status::Sent; // FIXME(fpohtmeh): need this?
        m_models->messages()->writeMessage(*message);
        m_userDatabase->writeMessage(*message);
    }
    else {
        // FIXME(fpohtmeh): change unread count
        // senderId != m_recipientId;
        if (!m_models->chats()->hasChatWithContact(senderId)) {
            m_models->chats()->createChat(senderId);
        }
        m_models->messages()->writeMessage(*message);
        m_userDatabase->writeMessage(*message);
    }
}

void MessagesController::sendMessage(const Message &message)
{
    QtConcurrent::run([=]() -> void {
        qDebug() << "Message sending started:" << message.id;
        auto m = message;
        if (m.attachment) {
            qCDebug(lcMessenger) << "Trying to upload the attachment";
// TODO(fpohtmeh): fix logic below
//            m.attachment = uploadAttachment(m);
//            if (!m.attachment) {
//                qCDebug(lcMessenger) << "Attachment was NOT uploaded";
//                emit messageStatusChanged(message.id, m_recipientId, Message::Status::Sent, QPrivateSignal());
//                return true;
//            }
            qCDebug(lcMessenger) << "Everything was uploaded. Continue to send message";
        }

        QMutexLocker _guard(&m_messageGuard);
        const auto encryptedStr = Core::encryptMessage(message, m_recipientId);
        if (!encryptedStr) {
            emit messageStatusChanged(message.id, m_recipientId, Message::Status::Invalid, QPrivateSignal());
        }
        else {
            const auto toJID = Utils::createJid(m_recipientId, m_messenger->xmppURL());
            const auto fromJID = Utils::createJid(m_userId, m_messenger->xmppURL());

            QXmppMessage msg(fromJID, toJID, *encryptedStr);
            msg.setReceiptRequested(true);
            msg.setId(message.id);

            // Send message and update status
            if (m_messenger->xmpp()->sendPacket(msg)) {
                qCDebug(lcMessenger) << "Message sent:" << message.id;
                emit messageStatusChanged(message.id, m_recipientId, Message::Status::Sent, QPrivateSignal());
            } else {
                emit messageStatusChanged(message.id, m_recipientId, Message::Status::Failed, QPrivateSignal());
            }
        }
    });
}

void MessagesController::sendNotSentMessages()
{
    // FIXME(fpohtmeh): implement
    // Get not sent messages
    // Send them but don't create again
}
