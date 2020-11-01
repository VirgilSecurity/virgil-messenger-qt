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
    // User database
    connect(userDatabase, &UserDatabase::opened, this, &MessagesController::setupTableConnections);
    connect(userDatabase, &UserDatabase::usernameChanged, this, &MessagesController::setUserId);
    // Chat
    connect(m_models->chats(), &ChatsModel::chatUpdated, this, &MessagesController::processUpdatedChat);
    // Status
    connect(this, &MessagesController::setMessageStatus, this, &MessagesController::processSetMessageStatus);
    // Network
    connect(messenger, &VSQMessenger::fireReady, this, &MessagesController::sendNotSentMessages);
    // Xmpp connections
    auto xmpp = messenger->xmpp();
    connect(xmpp, &QXmppClient::messageReceived, this, &MessagesController::receiveMessage);
    auto receipt = xmpp->findExtension<QXmppMessageReceiptManager>();
    connect(receipt, &QXmppMessageReceiptManager::messageDelivered, this, &MessagesController::setDeliveredStatus);
    auto carbon = xmpp->findExtension<QXmppCarbonManager>();
    connect(carbon, &QXmppCarbonManager::messageReceived, xmpp, &QXmppClient::messageReceived); // sent to our account (forwarded from another client)
    connect(carbon, &QXmppCarbonManager::messageSent, xmpp, &QXmppClient::messageReceived); // sent from our account (but another client)
}

void MessagesController::loadMessages(const Chat &chat)
{
    m_chat = chat;
    m_messenger->setCurrentRecipient(chat.contactId);
    if (chat.id.isEmpty()) {
        m_models->messages()->setMessages({});
    }
    else {
        m_userDatabase->messagesTable()->fetch(chat.id);
    }
}

void MessagesController::createSendMessage(const QString &body, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType)
{
    if (body.isEmpty() && !attachmentUrl.isValid()) {
        qDebug() << "Text and attachment are empty";
        return;
    }
    const auto attachment = m_models->attachments()->createAttachment(attachmentUrl.toUrl(), attachmentType);
    auto message = m_models->messages()->createMessage(m_chat.id, m_userId, body, attachment);
    const Chat::UnreadCount unreadCount = 0; // message can be created in current chat only
    m_models->chats()->updateLastMessage(message, unreadCount);
    m_userDatabase->writeMessage(message, unreadCount);
    sendMessage(message);
    emit messageAdded(message);
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

void MessagesController::processUpdatedChat(const Chat &chat)
{
    if (chat.id == m_chat.id) {
        m_chat = chat;
    }
}

void MessagesController::processSetMessageStatus(const Message::Id &messageId, const Contact::Id &contactId, const Message::Status &status)
{
    qDebug() << "Set message status:" << messageId << "contact" << contactId << "status" << status;
    if (status == Message::Status::Read) {
        qWarning() << "Marking of single message as read isn't supported yet";
    }
    else if (status == Message::Status::Created) {
        qWarning() << "Set message status to created isn't allowed";
    }
    else {
        if (contactId == m_chat.contactId) {
            m_models->messages()->setMessageStatus(messageId, status);
        }
        m_models->chats()->updateLastMessageStatus(messageId, status);
        m_userDatabase->messagesTable()->updateStatus(messageId, status);
        emit messageStatusChanged(messageId, contactId, status);
    }
}

void MessagesController::setDeliveredStatus(const Jid &jid, const Message::Id &messageId)
{
    setMessageStatus(messageId, Utils::contactIdFromJid(jid), Message::Status::Delivered, QPrivateSignal());
}

void MessagesController::receiveMessage(const QXmppMessage &msg)
{
    const auto senderId = Utils::contactIdFromJid(msg.from());
    const auto recipientId = Utils::contactIdFromJid(msg.to());
    const auto timestamp = QDateTime::currentDateTime();
    qInfo() << "Received message from:" << senderId << "recipient:" << recipientId;
    // Decrypt message
    auto decryptedMessage = Core::decryptMessage(senderId, msg.body());
    if (!decryptedMessage) {
        return;
    }
    auto message = *decryptedMessage;
    message.id = msg.id();
    message.timestamp = timestamp;

    const auto messages = m_models->messages();
    const auto chats = m_models->chats();
    auto chat = m_chat;
    bool isNewChat = false;
    if (senderId == m_userId) {
        message.status = Message::Status::Sent;
        if (recipientId == m_chat.contactId) {
            qDebug() << "Received carbon message to current chat";
        }
        else {
            qDebug() << "Received carbon message to not current chat";
            auto senderChat = chats->findByContact(recipientId);
            if (senderChat) {
                chat = *senderChat;
            }
            else {
                chat = chats->createChat(recipientId);
                isNewChat = true;
            }
        }
    }
    else {
        if (senderId == m_chat.contactId) {
            qDebug() << "Received a message to current chat";
        }
        else {
            qDebug() << "Received a message to not current chat";
            auto senderChat = chats->findByContact(senderId);
            if (senderChat) {
                chat = *senderChat;
            }
            else {
                chat = chats->createChat(senderId);
                isNewChat = true;
            }
            ++chat.unreadMessageCount;
        }
    }
    message.chatId = chat.id;
    messages->writeMessage(message);
    chats->updateLastMessage(message, chat.unreadMessageCount);
    if (isNewChat) {
        chat.lastMessage = message;
        m_userDatabase->writeChatAndLastMessage(chat);
    }
    else {
        m_userDatabase->writeMessage(message, chat.unreadMessageCount);
    }
    emit messageAdded(message);
}

void MessagesController::sendMessage(const Message &message)
{
    QtConcurrent::run([=]() -> void {
        qDebug() << "Message sending started:" << message.id;
        auto m = message;
        if (m.attachment) {
            qCDebug(lcMessenger) << "Trying to upload the attachment";
            // TODO(fpohtmeh): uploadAttachment() here
            qCDebug(lcMessenger) << "Everything was uploaded. Continue to send message";
        }

        QMutexLocker _guard(&m_messageGuard);
        const auto encryptedStr = Core::encryptMessage(message, m_chat.contactId);
        if (!encryptedStr) {
            emit setMessageStatus(message.id, m_chat.contactId, Message::Status::InvalidM, QPrivateSignal());
        }
        else {
            const auto toJID = Utils::createJid(m_chat.contactId, m_messenger->xmppURL());
            const auto fromJID = Utils::createJid(m_userId, m_messenger->xmppURL());

            QXmppMessage msg(fromJID, toJID, *encryptedStr);
            msg.setReceiptRequested(true);
            msg.setId(message.id);

            // Send message and update status
            if (m_messenger->xmpp()->sendPacket(msg)) {
                qCDebug(lcMessenger) << "Message sent:" << message.id;
                emit setMessageStatus(message.id, m_chat.contactId, Message::Status::Sent, QPrivateSignal());
            } else {
                emit setMessageStatus(message.id, m_chat.contactId, Message::Status::Failed, QPrivateSignal());
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
