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

#include "Core.h"
#include "VSQMessenger.h"
#include "Utils.h"
#include "controllers/ChatsController.h"
#include "database/AttachmentsTable.h"
#include "database/MessagesTable.h"
#include "database/UserDatabase.h"
#include "models/AttachmentsModel.h"
#include "models/ChatsModel.h"
#include "models/MessagesModel.h"
#include "models/MessagesQueue.h"
#include "models/Models.h"

using namespace vm;

MessagesController::MessagesController(VSQMessenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
    , m_models(models)
    , m_userDatabase(userDatabase)
{
    auto messagesQueue = m_models->messagesQueue();
    // User database
    connect(userDatabase, &UserDatabase::opened, this, &MessagesController::setupTableConnections);
    // Queue
    connect(this, &MessagesController::messageCreated, messagesQueue, &MessagesQueue::pushMessage);
    connect(messagesQueue, &MessagesQueue::messageStatusChanged, this, &MessagesController::setMessageStatus);
    connect(messagesQueue, &MessagesQueue::attachmentStatusChanged, this, &MessagesController::setAttachmentStatus);
    connect(messagesQueue, &MessagesQueue::attachmentUrlChanged, this, &MessagesController::setAttachmentUrl);
    connect(messagesQueue, &MessagesQueue::attachmentLocalPathChanged, this, &MessagesController::setAttachmentLocalPath);
    connect(messagesQueue, &MessagesQueue::attachmentFingerprintChanged, this, &MessagesController::setAttachmentFingerprint);
    connect(messagesQueue, &MessagesQueue::attachmentExtrasChanged, this, &MessagesController::setAttachmentExtras);
    connect(messagesQueue, &MessagesQueue::attachmentProcessedSizeChanged, this, &MessagesController::setAttachmentProcessedSize);
    connect(messagesQueue, &MessagesQueue::attachmentEncryptedSizeChanged, this, &MessagesController::setAttachmentEncryptedSize);
    // Models
    connect(m_models->chats(), &ChatsModel::chatUpdated, this, &MessagesController::onChatUpdated);
    connect(m_models->messages(), &MessagesModel::displayImageNotFound, this, &MessagesController::displayImageNotFound);
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
        m_userDatabase->messagesTable()->fetchChatMessages(chat.id);
    }
}

void MessagesController::createSendMessage(const QString &body, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType)
{
    const auto isAttachment = attachmentUrl.isValid();
    if (body.isEmpty() && !isAttachment) {
        qCWarning(lcController) << "Text and attachment are empty";
        return;
    }
    const auto attachment = m_models->attachments()->createAttachment(attachmentUrl.toUrl(), attachmentType);
    if (isAttachment && !attachment) {
        qCWarning(lcController) << "Message wasn't created. Attachment is invalid";
        emit notificationCreated(tr("Attachment reading error"));
        return;
    }
    auto message = m_models->messages()->createMessage(m_chat.id, m_userId, body, attachment);
    const Chat::UnreadCount unreadCount = 0; // message can be created in current chat only
    m_models->chats()->updateLastMessage(message, unreadCount);
    m_userDatabase->writeMessage(message, unreadCount);
    emit messageCreated({ message, m_userId, m_chat.contactId, m_userId, m_chat.contactId });
}

void MessagesController::setupTableConnections()
{
    auto table = m_userDatabase->messagesTable();
    connect(table, &MessagesTable::errorOccurred, this, &MessagesController::errorOccurred);
    connect(table, &MessagesTable::chatMessagesFetched, m_models->messages(), &MessagesModel::setMessages);
}

void MessagesController::setUserId(const UserId &userId)
{
    m_userId = userId;
    qCDebug(lcController) << "Set messages controller userId:" << userId;
    m_models->messages()->setUserId(userId);
    m_models->messagesQueue()->setUserId(userId);

    if (!m_userId.isEmpty()) {
        m_postponedXmppData.process(this);
    }
}

void MessagesController::onChatUpdated(const Chat &chat)
{
    if (chat.id == m_chat.id) {
        m_chat = chat;
    }
}

void MessagesController::setMessageStatus(const Message::Id &messageId, const Contact::Id &contactId, const Message::Status &status)
{
    qCDebug(lcController) << "Set message status:" << messageId << "contact" << contactId << "status" << status;
    if (status == Message::Status::Read) {
        qCWarning(lcModel) << "Marking of single message as read isn't supported yet";
    }
    else if (status == Message::Status::Created) {
        qCWarning(lcModel) << "Set message status to created isn't allowed";
    }
    else {
        if (contactId == m_chat.contactId) {
            m_models->messages()->setMessageStatus(messageId, status);
        }
        m_userDatabase->messagesTable()->updateStatus(messageId, status);
        emit messageStatusChanged(messageId, contactId, status);
    }
}

void MessagesController::setDeliveredStatus(const Jid &jid, const Message::Id &messageId)
{
    if (m_userId.isEmpty()) {
        m_postponedXmppData.addDeliverInfo(jid, messageId);
    }
    else {
        setMessageStatus(messageId, Utils::contactIdFromJid(jid), Message::Status::Delivered);
    }
}

void MessagesController::setAttachmentStatus(const Attachment::Id &attachmentId, const Contact::Id &contactId, const Attachment::Status &status)
{
    qCDebug(lcController) << "Set attachment status:" << attachmentId << "contact" << contactId << "status" << status;
    if (contactId == m_chat.contactId) {
        m_models->messages()->setAttachmentStatus(attachmentId, status);
    }
    m_userDatabase->attachmentsTable()->updateStatus(attachmentId, status);
}

void MessagesController::setAttachmentUrl(const Attachment::Id &attachmentId, const Contact::Id &contactId, const QUrl &url)
{
    qCDebug(lcController) << "Set attachment url:" << attachmentId << "contact" << contactId << "url filename" << url.fileName();
    if (contactId == m_chat.contactId) {
        m_models->messages()->setAttachmentUrl(attachmentId, url);
    }
    m_userDatabase->attachmentsTable()->updateUrl(attachmentId, url);
}

void MessagesController::setAttachmentExtras(const Attachment::Id &attachmentId, const Contact::Id &contactId, const Attachment::Type &type, const QVariant &extras)
{
    qCDebug(lcController) << "Set attachment extras:" << attachmentId << "contact" << contactId;
    if (contactId == m_chat.contactId) {
        m_models->messages()->setAttachmentExtras(attachmentId, extras);
    }
    m_userDatabase->attachmentsTable()->updateExtras(attachmentId, type, extras);
}

void MessagesController::setAttachmentProcessedSize(const Attachment::Id &attachmentId, const Contact::Id &contactId, const DataSize &processedSize)
{
    //qCDebug(lcController) << "Set attachment processed size:" << attachmentId << "contact" << contactId << "processed size" << processedSize;
    if (contactId == m_chat.contactId) {
        m_models->messages()->setAttachmentProcessedSize(attachmentId, processedSize);
    }
}

void MessagesController::setAttachmentEncryptedSize(const Attachment::Id &attachmentId, const Contact::Id &contactId, const DataSize &encryptedSize)
{
    qCDebug(lcController) << "Set attachment encrypted size:" << attachmentId << "contact" << contactId << "encrypted size" << encryptedSize;
    if (contactId == m_chat.contactId) {
        m_models->messages()->setAttachmentEncryptedSize(attachmentId, encryptedSize);
    }
    m_userDatabase->attachmentsTable()->updateEncryptedSize(attachmentId, encryptedSize);
}

void MessagesController::setAttachmentLocalPath(const Attachment::Id &attachmentId, const Contact::Id &contactId, const QString &localPath)
{
    qCDebug(lcController) << "Set attachment local path:" << attachmentId << "contact" << contactId << "path" << localPath;
    if (contactId == m_chat.contactId) {
        m_models->messages()->setAttachmentLocalPath(attachmentId, localPath);
    }
    m_userDatabase->attachmentsTable()->updateLocalPath(attachmentId, localPath);
}

void MessagesController::setAttachmentFingerprint(const Attachment::Id &attachmentId, const Contact::Id &contactId, const QString &fingerpint)
{
    qCDebug(lcController) << "Set attachment fingerprint:" << attachmentId << "contact" << contactId << "fingerprint" << fingerpint;
    if (contactId == m_chat.contactId) {
        m_models->messages()->setAttachmentFingerprint(attachmentId, fingerpint);
    }
    m_userDatabase->attachmentsTable()->updateFingerprint(attachmentId, fingerpint);
}

void MessagesController::receiveMessage(const QXmppMessage &msg)
{
    if (m_userId.isEmpty()) {
        m_postponedXmppData.addMessage(msg);
        return;
    }

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
    if (message.attachment) {
        message.attachment->messageId = message.id;
    }

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

void MessagesController::PostponedXmppData::addMessage(const QXmppMessage &msg)
{
    qCDebug(lcController) << "Collecting of xmpp message from" << msg.from();
    receivedMessages.push_back(msg);
}

void MessagesController::PostponedXmppData::addDeliverInfo(const Jid jid, const Message::Id messageId)
{
    qCDebug(lcController) << "Collecting of xmpp delivered status from" << jid;
    deliverInfos.push_back({ jid, messageId });
}

void MessagesController::PostponedXmppData::process(MessagesController *controller)
{
    for (auto &m : receivedMessages) {
        qCDebug(lcController) << "Processing of xmpp message from" << m.from();
        controller->receiveMessage(m);
    }
    receivedMessages.clear();

    for (auto &m : deliverInfos) {
        qCDebug(lcController) << "Processing of xmpp delivered status from" << m.jid;
        controller->setDeliveredStatus(m.jid, m.messageId);
    }
    deliverInfos.clear();
}
