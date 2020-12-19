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
#include <qxmpp/QXmppMessageReceiptManager.h>

#include "Messenger.h"
#include "Utils.h"
#include "controllers/ChatsController.h"
#include "database/AttachmentsTable.h"
#include "database/MessagesTable.h"
#include "database/UserDatabase.h"
#include "models/ChatsModel.h"
#include "models/MessagesModel.h"
#include "models/MessagesQueue.h"
#include "models/Models.h"
#include "Controller.h"

using namespace vm;
using Self = MessagesController;

Self::MessagesController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
    , m_models(models)
    , m_userDatabase(userDatabase)
{
    auto messagesQueue = m_models->messagesQueue();
    // User database
    connect(userDatabase, &UserDatabase::opened, this, &Self::setupTableConnections);
    // Queue
    connect(this, &Self::messageCreated, messagesQueue, &MessagesQueue::pushMessage);
    connect(messagesQueue, &MessagesQueue::messageChanged, this, &Self::onMessageUpdated);
    // Models
    connect(m_models->chats(), &ChatsModel::chatUpdated, this, &Self::onChatUpdated);
    connect(m_models->messages(), &MessagesModel::displayImageNotFound, this, &Self::displayImageNotFound);
    // Messages
    connect(m_messenger, &Messenger::messageReceived, this, &Self::onMessageReceived);
    connect(m_messenger, &Messenger::messageUpdated, this, &Self::onMessageUpdated);
}

void Self::loadMessages(const ChatHandler &chat)
{
    m_chat = chat;
    m_userDatabase->messagesTable()->fetchChatMessages(chat->id());
}

void Self::clearMessages()
{
    m_chat = nullptr;
    m_models->messages()->cleartChat();
}

void Self::sendTextMessage(const QString &body)
{
    auto message = m_models->messages()->createTextMessage(body);
    const qsizetype unreadCount = 0; // message can be created in current chat only
    m_models->chats()->updateLastMessage(message, unreadCount);
    m_userDatabase->writeMessage(*message, unreadCount);
    emit messageCreated(message);
}

void Self::sendFileMessage(const QVariant &attachmentUrl)
{
    if (!attachmentUrl.isValid()) {
        qCWarning(lcController) << "File attachment URL is empty";
        return;
    }

    auto message = m_models->messages()->createFileMessage(attachmentUrl.toUrl());
    if (message) {
        qCWarning(lcController) << "Message wasn't created. File is invalid.";
        emit notificationCreated(tr("File reading error"), true);
        return;
    }

    const qsizetype unreadCount = 0; // message can be created in current chat only
    m_models->chats()->updateLastMessage(message, unreadCount);
    m_userDatabase->writeMessage(message, unreadCount);
    emit messageCreated(message);
}

void Self::sendPictureMessage(const QVariant &attachmentUrl)
{
    if (!attachmentUrl.isValid()) {
        qCWarning(lcController) << "Picture attachment URL is empty";
        return;
    }

    auto message = m_models->messages()->createPictureMessage(attachmentUrl.toUrl());
    if (message) {
        qCWarning(lcController) << "Message wasn't created. Picture is invalid.";
        emit notificationCreated(tr("Picture reading error"), true);
        return;
    }

    const qsizetype unreadCount = 0; // message can be created in current chat only
    m_models->chats()->updateLastMessage(message, unreadCount);
    m_userDatabase->writeMessage(message, unreadCount);
    emit messageCreated(message);
}

void Self::setupTableConnections()
{
    auto table = m_userDatabase->messagesTable();
    connect(table, &MessagesTable::errorOccurred, this, &Self::errorOccurred);
    connect(table, &MessagesTable::chatMessagesFetched, m_models->messages(), &MessagesModel::setMessages);
}

void Self::setUserId(const UserId &userId)
{
    m_userId = userId;
    qCDebug(lcController) << "Set messages controller userId:" << userId;
    m_models->messages()->setUserId(userId);
    m_models->messagesQueue()->setUserId(userId);

    if (!m_userId.isEmpty()) {
        m_postponedMessages.process(this);
    }
}

void Self::onChatUpdated(const Chat &chat)
{
    if (chat.id == m_chat.id) {
        m_chat = chat;
    }
}

void Self::onMessageUpdated(const MessageUpdate& messageUpdate)
{
    //
    //  Update DB.
    //
    m_userDatabase->messagesTable()->updateMessage(messageUpdate);

    //
    //  Update UI for the current chat.
    //
    if (message.chatId() == m_chat.id()) {
        const bool isUpdated = m_models->messages()->applyMessageUpdate(messageUpdate);
        if (isUpdated) {
            emit messageUpdated(messageUpdate);
        }
    }
}

void Self::onMessageReceived(MessageHandler message)
{
    const auto senderId = message.senderId;
    const auto recipientId = message.recipientId;
    const auto timestamp = message.timestamp;
    qInfo() << "Received message from:" << senderId << "recipient:" << recipientId;

    const auto messages = m_models->messages();
    const auto chats = m_models->chats();
    auto chat = m_chat;
    bool isNewChat = false;
    if (senderId == m_userId) {
        message.status = Message::Status::Sent;
        if (recipientId == m_chat.contactId) {
            qCDebug(lcController) << "Received carbon message to current chat";
        }
        else {
            qCDebug(lcController) << "Received carbon message to not current chat";
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
            qCDebug(lcController) << "Received a message to current chat";
        }
        else {
            qCDebug(lcController) << "Received a message to not current chat";
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
    if (m_chat.id == chat.id) {
        messages->writeMessage(message);
    }
    chats->updateLastMessage(message, chat.unreadMessageCount);
    if (isNewChat) {
        chat.lastMessage = message;
        m_userDatabase->writeChatAndLastMessage(chat);
    }
    else {
        m_userDatabase->writeMessage(message, chat.unreadMessageCount);
    }
    emit messageCreated({ message, m_userId, chat.contactId, senderId, recipientId });
}

void Self::onMessageUpdated(const MessageUpadte &messageUpdate) {

}
