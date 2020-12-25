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
#include "OutgoingMessage.h"

using namespace vm;
using Self = MessagesController;

Self::MessagesController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
    , m_models(models)
    , m_userDatabase(userDatabase)
{
    qRegisterMetaType<MessageHandler>("MessageHandler");
    qRegisterMetaType<MessageUpdate>("MessageUpdate");

    auto messagesQueue = m_models->messagesQueue();
    // User database
    connect(userDatabase, &UserDatabase::opened, this, &Self::setupTableConnections);
    // Queue
    connect(this, &Self::messageCreated, messagesQueue, &MessagesQueue::pushMessage);
    connect(messagesQueue, &MessagesQueue::updateMessage, this, &Self::onUpdateMessage);
    // Models
    connect(m_models->messages(), &MessagesModel::displayImageNotFound, this, &Self::displayImageNotFound);
    // Messages
    connect(m_messenger, &Messenger::messageReceived, this, &Self::onMessageReceived);
    connect(m_messenger, &Messenger::updateMessage, this, &Self::onUpdateMessage);
}

void Self::loadMessages(const ChatHandler &chat)
{
    m_models->messages()->setChat(chat);
    m_userDatabase->messagesTable()->fetchChatMessages(chat->id());
}

void Self::clearMessages()
{
    m_models->messages()->clearChat();
}

void Self::sendTextMessage(const QString &body)
{
    auto message = createTextMessage(body);
    const qsizetype unreadCount = 0; // message can be created in current chat only
    m_userDatabase->writeMessage(message, unreadCount);
    m_models->chats()->updateLastMessage(message, unreadCount);
    m_models->messages()->addMessage(message);
    emit messageCreated(message);
}

void Self::sendFileMessage(const QVariant &attachmentUrl)
{
    if (!attachmentUrl.isValid()) {
        qCWarning(lcController) << "File attachment URL is empty";
        return;
    }

    auto message = createFileMessage(attachmentUrl.toUrl());
    if (!message) {
        qCWarning(lcController) << "Message wasn't created. File is invalid.";
        emit notificationCreated(tr("File reading error"), true);
        return;
    }

    const qsizetype unreadCount = 0; // message can be created in current chat only
    m_models->chats()->updateLastMessage(message, unreadCount);
    m_userDatabase->writeMessage(message, unreadCount);
    m_models->messages()->addMessage(message);
    emit messageCreated(message);
}

void Self::sendPictureMessage(const QVariant &attachmentUrl)
{
    if (!attachmentUrl.isValid()) {
        qCWarning(lcController) << "Picture attachment URL is empty";
        return;
    }

    auto message = createPictureMessage(attachmentUrl.toUrl());
    if (!message) {
        qCWarning(lcController) << "Message wasn't created. Picture is invalid.";
        emit notificationCreated(tr("Picture reading error"), true);
        return;
    }

    const qsizetype unreadCount = 0; // message can be created in current chat only
    m_models->chats()->updateLastMessage(message, unreadCount);
    m_userDatabase->writeMessage(message, unreadCount);
    m_models->messages()->addMessage(message);
    emit messageCreated(message);
}


std::unique_ptr<OutgoingMessage> Self::createOutgoingMessage() {
    auto message = std::make_unique<OutgoingMessage>();

    auto currentChat = m_models->messages()->chat();

    message->setId(MessageId::generate());
    message->setChatId(currentChat->id());
    message->setChatType(currentChat->type());
    message->setStage(OutgoingMessageStage::Created);
    message->setCreatedAt(QDateTime::currentDateTime());
    message->setSenderId(m_messenger->currentUser()->id());

    return message;
}


ModifiableMessageHandler Self::createTextMessage(const QString &body)
{
    auto content = MessageContentText(body);

    auto message = createOutgoingMessage();
    message->setContent(std::move(content));

    return message;
}


ModifiableMessageHandler Self::createFileMessage(const QUrl &localFileUrl)
{
    QString errorString;
    auto content = MessageContentFile::createFromLocalFile(localFileUrl, errorString);
    // FIXME(fpohtmeh): is it correct way to handle errors? skip errorString?
    if (!errorString.isEmpty()) {
        return ModifiableMessageHandler();
    }

    auto message = createOutgoingMessage();
    message->setContent(std::move(content));

    return message;
}


ModifiableMessageHandler Self::createPictureMessage(const QUrl &localFileUrl)
{
    QString errorString;
    const auto content = MessageContentPicture::createFromLocalFile(localFileUrl, errorString);
    // FIXME(fpohtmeh): is it correct way to handle errors? skip errorString?
    if (!errorString.isEmpty()) {
        return ModifiableMessageHandler();
    }

    auto message = createOutgoingMessage();
    message->setContent(std::move(content));

    return message;
}


void Self::setupTableConnections()
{
    auto table = m_userDatabase->messagesTable();
    connect(table, &MessagesTable::errorOccurred, this, &Self::errorOccurred);
    connect(table, &MessagesTable::chatMessagesFetched, m_models->messages(), &MessagesModel::setMessages);
}

void Self::onUpdateMessage(const MessageUpdate& messageUpdate)
{
    //
    //  Update DB.
    //
    m_userDatabase->messagesTable()->updateMessage(messageUpdate);

    //
    //  Update UI for the current chat.
    //
    if (m_models->messages()->updateMessage(messageUpdate)) {
        // TODO: Check if this signal is still needed.
        emit updateMessage(messageUpdate);
    }
}

void Self::onMessageReceived(ModifiableMessageHandler message)
{

    //
    //  Got a new message.
    //
    qInfo(lcController()) << "Received message from:" << message->senderId() << "recipient:" << message->recipientId();

    auto messages = m_models->messages();
    auto chats = m_models->chats();

    //
    // Find destination chat.
    //
    auto destChat = chats->findChat(message->chatId());
    if (destChat) {
        //
        //  Update existing chat.
        //
        destChat->setLastMessage(message);
        m_userDatabase->writeMessage(message, destChat->unreadMessageCount() + 1);
    } else {
        //
        //  Create a new chat.
        //
        destChat = std::make_unique<Chat>();
        destChat->setId(message->chatId());
        destChat->setTitle(message->chatId());
        destChat->setCreatedAt(QDateTime::currentDateTime());
        destChat->setLastMessage(message);
        destChat->setType(message->isGroupChatMessage() ? ChatType::Group : ChatType::Personal);
        chats->addChat(destChat);

        m_userDatabase->writeChatAndLastMessage(destChat);
    }

    messages->addMessage(message);

    emit messageCreated(message);
}
