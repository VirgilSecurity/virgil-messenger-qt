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

#include "Messenger.h"

#include <QThread>

#include "Settings.h"
#include "Utils.h"
#include "VirgilCore.h"
#include "database/Database.h"

Messenger::Messenger(Settings *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_messageModel(this)
    , m_chatsModel(this)
    , m_database(new Database(settings, nullptr))
    , m_databaseThread(new QThread())
    , m_client(new Client(settings, nullptr))
    , m_clientThread(new QThread())
{
    m_database->moveToThread(m_databaseThread);
    connect(m_databaseThread, &QThread::started, m_database, &Database::open);
    connect(m_database, &Database::failed, this, &Messenger::quitRequested);
    m_databaseThread->start();

    m_client->moveToThread(m_clientThread);
    connect(m_clientThread, &QThread::started, m_client, &Client::start);
    m_clientThread->start();

#if VS_PUSHNOTIFICATIONS
    VSQPushNotifications::instance().startMessaging();
#endif
}

void Messenger::start()
{
    setupConnections();

    // Sign-in or request credentials
    if (m_settings->lastSignedInUser().isEmpty())
        emit credentialsRequested(false);
    else
        signIn(m_settings->lastSignedInUser());
}

void Messenger::setupConnections()
{
    // Authorization: messenger-to-client
    connect(this, &Messenger::signIn, m_client, &Client::signIn);
    connect(this, &Messenger::signInWithKey, m_client, &Client::signInWithKey);
    connect(this, &Messenger::signOut, m_client, &Client::signOut);
    connect(this, &Messenger::signUp, m_client, &Client::signUp);
    connect(this, &Messenger::backupKey, m_client, &Client::backupKey);
    // Authorization status: client-to-messenger
    connect(m_client, &Client::signedIn, this, &Messenger::signedIn);
    connect(m_client, &Client::signedIn, this, &Messenger::setUser);
    connect(m_client, &Client::signInFailed, this, &Messenger::signInFailed);
    connect(m_client, &Client::signedOut, this, std::bind(&Messenger::credentialsRequested, this, true));
    connect(m_client, &Client::signedOut, this, std::bind(&Messenger::setUser, this, QLatin1String()));
    connect(m_client, &Client::signedOut, this, &Messenger::signedOut);
    connect(m_client, &Client::signedUp, this, &Messenger::signedUp);
    connect(m_client, &Client::signUpFailed, this, &Messenger::signUpFailed);
    connect(m_client, &Client::keyBackuped, this, &Messenger::keyBackuped);
    connect(m_client, &Client::backupKeyFailed, this, &Messenger::backupKeyFailed);
    // Signed users: client-to-settings
    connect(m_client, &Client::signedIn, m_settings, &Settings::addUserToList);
    connect(m_client, &Client::signedIn, m_settings, &Settings::setLastSignedInUser);
    connect(m_client, &Client::signedUp, m_settings, &Settings::addUserToList);
    connect(m_client, &Client::signedUp, m_settings, &Settings::setLastSignedInUser);

    // Contacts: messenger-to-client
    connect(this, &Messenger::addContact, m_client, &Client::addContact);
    // Contacts status: client-to-messenger
    connect(m_client, &Client::contactAdded, this, &Messenger::contactAdded);
    connect(m_client, &Client::addContactFailed, this, &Messenger::addContactFailed);
    // Contacts status: client-to-models
    connect(m_client, &Client::contactAdded, &m_chatsModel, &ChatsModel::processContact);

    // Messages: messenger-to-client
    connect(this, &Messenger::createSendMessage, this, &Messenger::onCreateSendMessage);
    connect(this, &Messenger::sendMessage, m_client, &Client::sendMessage);
    // Messages status: client-to-messenger
    connect(m_client, &Client::messageSent, this, &Messenger::messageSent);
    connect(m_client, &Client::sendMessageFailed, this, &Messenger::sendMessageFailed);
    // Messages: messenger-to-models
    connect(this, &Messenger::sendMessage, &m_messageModel, &MessagesModel::addMessage);
    // Messages status: client-to-model
    connect(m_client, &Client::messageReceived, &m_messageModel, &MessagesModel::addMessage);
    connect(m_client, &Client::messageSent, &m_messageModel,
        std::bind(&MessagesModel::setMessageStatus, &m_messageModel, std::placeholders::_1, Message::Status::Sent));
    connect(m_client, &Client::sendMessageFailed, &m_messageModel,
        std::bind(&MessagesModel::setMessageStatus, &m_messageModel, std::placeholders::_1, Message::Status::Failed));
    connect(m_client, &Client::messageDelivered, &m_messageModel,
        std::bind(&MessagesModel::setMessageStatusById, &m_messageModel, std::placeholders::_1, Message::Status::Received));
    // Messages status: model-to-model
    connect(&m_messageModel, &MessagesModel::messageAdded, &m_chatsModel, &ChatsModel::processMessage);
    connect(&m_messageModel, &MessagesModel::messageStatusChanged, &m_chatsModel, &ChatsModel::updateMessageStatus);

    // Other connections: messenger-to-client
    connect(this, &Messenger::checkConnectionState, m_client, &Client::checkConnectionState);
    connect(this, &Messenger::setOnlineStatus, m_client, &Client::setOnlineStatus);
    // Other connections: messenger-to-models
    connect(this, &Messenger::userChanged, &m_messageModel, &MessagesModel::setUser);
    connect(this, &Messenger::recipientChanged, &m_messageModel, &MessagesModel::setRecipient);
    connect(this, &Messenger::recipientChanged, &m_chatsModel, &ChatsModel::setRecipient);
}

void Messenger::setUser(const QString &user)
{
    if (user == m_user)
        return;
    m_user = user;
    emit userChanged(user);

    setRecipient(QString());
}

void Messenger::setRecipient(const QString &recipient)
{
    if (recipient == m_recipient)
        return;
    m_recipient = recipient;
    emit recipientChanged(recipient);
}

MessagesModel *Messenger::messageModel()
{
    return &m_messageModel;
}

ChatsModel *Messenger::chatsModel()
{
    return &m_chatsModel;
}

void Messenger::onCreateSendMessage(const QString &text, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType)
{
    // TODO(fpohtmeh): create from atttachment
    Q_UNUSED(attachmentUrl);
    Q_UNUSED(attachmentType);
    Optional<Attachment> attachment;
    // Create message
    const QString uuid = Utils::createUuid();
    QString messageText = text;
    if (attachment)
        messageText = attachment->fileName();
    const Message message {
        uuid,
        QDateTime::currentDateTime(),
        messageText,
        m_recipient,
        Message::Author::User,
        attachment,
        Message::Status::Created
    };
    // Emit
    emit sendMessage(message);
}
