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

#include "VSQMessenger.h"

#include <QThread>

#include "VSQCrashReporter.h"
#include "VSQPushNotifications.h"
#include "VSQSettings.h"
#include "VSQUtils.h"
#include "client/VSQCore.h"
#include "database/VSQDatabase.h"

VSQMessenger::VSQMessenger(VSQSettings *settings, VSQCrashReporter *crashReporter, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_crashReporter(crashReporter)
    , m_attachmentBuilder(settings)
    , m_messageModel(this)
    , m_chatsModel(this)
    , m_database(new VSQDatabase(settings, nullptr))
    , m_databaseThread(new QThread())
    , m_client(new VSQClient(settings, nullptr))
    , m_clientThread(new QThread())
{
    m_database->moveToThread(m_databaseThread);
    connect(m_databaseThread, &QThread::started, m_database, &VSQDatabase::open);
    connect(m_database, &VSQDatabase::failed, this, &VSQMessenger::quitRequested);
    m_databaseThread->start();

    m_client->moveToThread(m_clientThread);
    connect(m_clientThread, &QThread::started, m_client, &VSQClient::start);
    m_clientThread->start();

#if VS_PUSHNOTIFICATIONS
    VSQPushNotifications::instance().startMessaging();
#endif
}

VSQMessenger::~VSQMessenger()
{
    m_clientThread->quit();
    m_clientThread->wait();
    delete m_client;
    delete m_clientThread;

    m_databaseThread->quit();
    m_databaseThread->wait();
    delete m_database;
    delete m_databaseThread;

#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Messenger";
#endif
}

void VSQMessenger::start()
{
    setupConnections();

    // Sign-in or request credentials
    if (m_settings->lastSignedInUser().isEmpty())
        emit credentialsRequested(false);
    else
        signIn(m_settings->lastSignedInUser());
}

void VSQMessenger::setupConnections()
{
    // Authorization: messenger-to-client
    connect(this, &VSQMessenger::signIn, m_client, &VSQClient::signIn);
    connect(this, &VSQMessenger::signInWithKey, m_client, &VSQClient::signInWithKey);
    connect(this, &VSQMessenger::signOut, m_client, &VSQClient::signOut);
    connect(this, &VSQMessenger::signUp, m_client, &VSQClient::signUp);
    connect(this, &VSQMessenger::backupKey, m_client, &VSQClient::backupKey);
    // Authorization status: client-to-messenger
    connect(m_client, &VSQClient::signedIn, this, &VSQMessenger::signedIn);
    connect(m_client, &VSQClient::signedIn, this, &VSQMessenger::setUser);
    connect(m_client, &VSQClient::signInFailed, this, &VSQMessenger::signInFailed);
    connect(m_client, &VSQClient::signedOut, this, std::bind(&VSQMessenger::credentialsRequested, this, true));
    connect(m_client, &VSQClient::signedOut, this, std::bind(&VSQMessenger::setUser, this, QLatin1String()));
    connect(m_client, &VSQClient::signedOut, this, &VSQMessenger::signedOut);
    connect(m_client, &VSQClient::signedUp, this, &VSQMessenger::signedUp);
    connect(m_client, &VSQClient::signUpFailed, this, &VSQMessenger::signUpFailed);
    connect(m_client, &VSQClient::keyBackuped, this, &VSQMessenger::keyBackuped);
    connect(m_client, &VSQClient::backupKeyFailed, this, &VSQMessenger::backupKeyFailed);
    // Signed users: client-to-settings
    connect(m_client, &VSQClient::signedIn, m_settings, &VSQSettings::addUserToList);
    connect(m_client, &VSQClient::signedIn, m_settings, &VSQSettings::setLastSignedInUser);
    connect(m_client, &VSQClient::signedUp, m_settings, &VSQSettings::addUserToList);
    connect(m_client, &VSQClient::signedUp, m_settings, &VSQSettings::setLastSignedInUser);

    // Contacts: messenger-to-client
    connect(this, &VSQMessenger::addContact, m_client, &VSQClient::addContact);
    // Contacts status: client-to-messenger
    connect(m_client, &VSQClient::contactAdded, this, &VSQMessenger::contactAdded);
    connect(m_client, &VSQClient::addContactFailed, this, &VSQMessenger::addContactFailed);
    // Contacts status: client-to-models
    connect(m_client, &VSQClient::contactAdded, &m_chatsModel, &VSQChatsModel::processContact);

    // Messages: messenger-to-client
    connect(this, &VSQMessenger::createSendMessage, this, &VSQMessenger::onCreateSendMessage);
    connect(this, &VSQMessenger::sendMessage, m_client, &VSQClient::sendMessage);
    // Messages status: client-to-messenger
    connect(m_client, &VSQClient::messageSent, this, &VSQMessenger::messageSent);
    connect(m_client, &VSQClient::sendMessageFailed, this, &VSQMessenger::sendMessageFailed);
    // Messages: messenger-to-models
    connect(this, &VSQMessenger::sendMessage, &m_messageModel, &VSQMessagesModel::addMessage);
    // Messages status: client-to-model
    connect(m_client, &VSQClient::messageReceived, &m_messageModel, &VSQMessagesModel::addMessage);
    connect(m_client, &VSQClient::messageSent, &m_messageModel,
        std::bind(&VSQMessagesModel::setMessageStatus, &m_messageModel, args::_1, Message::Status::Sent));
    connect(m_client, &VSQClient::sendMessageFailed, &m_messageModel,
        std::bind(&VSQMessagesModel::setMessageStatus, &m_messageModel, args::_1, Message::Status::Failed));
    connect(m_client, &VSQClient::messageDelivered, &m_messageModel,
        std::bind(&VSQMessagesModel::setMessageStatusById, &m_messageModel, args::_1, Message::Status::Received));

    // Upload status: client-to-model
    connect(m_client, &VSQClient::uploadStarted, &m_messageModel,
        std::bind(&VSQMessagesModel::setUploadFailed, &m_messageModel, args::_1, false));
    connect(m_client, &VSQClient::uploadProgressChanged, &m_messageModel, &VSQMessagesModel::setUploadProgress);
    connect(m_client, &VSQClient::uploaded, &m_messageModel,
        std::bind(&VSQMessagesModel::setUploadFailed, &m_messageModel, args::_1, false));
    connect(m_client, &VSQClient::uploadFailed, &m_messageModel,
        std::bind(&VSQMessagesModel::setUploadFailed, &m_messageModel, args::_1, true));
    // Messages status: model-to-model
    connect(&m_messageModel, &VSQMessagesModel::messageAdded, &m_chatsModel, &VSQChatsModel::processMessage);
    connect(&m_messageModel, &VSQMessagesModel::messageStatusChanged, &m_chatsModel, &VSQChatsModel::updateMessageStatus);

    // Other connections: client-to-crash reporter
    connect(m_client, &VSQClient::virgilUrlChanged, m_crashReporter, &VSQCrashReporter::setUrl);
    // Other connections: messenger-to-client
    connect(this, &VSQMessenger::checkConnectionState, m_client, &VSQClient::checkConnectionState);
    connect(this, &VSQMessenger::setOnlineStatus, m_client, &VSQClient::setOnlineStatus);
    // Other connections: messenger-to-models
    connect(this, &VSQMessenger::userChanged, &m_messageModel, &VSQMessagesModel::setUser);
    connect(this, &VSQMessenger::recipientChanged, &m_messageModel, &VSQMessagesModel::setRecipient);
    connect(this, &VSQMessenger::recipientChanged, &m_chatsModel, &VSQChatsModel::setRecipient);
}

void VSQMessenger::setUser(const QString &user)
{
    if (user == m_user)
        return;
    m_user = user;
    emit userChanged(user);

    setRecipient(QString());
}

void VSQMessenger::setRecipient(const QString &recipient)
{
    if (recipient == m_recipient)
        return;
    m_recipient = recipient;
    emit recipientChanged(recipient);
}

VSQMessagesModel *VSQMessenger::messageModel()
{
    return &m_messageModel;
}

VSQChatsModel *VSQMessenger::chatsModel()
{
    return &m_chatsModel;
}

void VSQMessenger::onCreateSendMessage(const QString &text, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType)
{
    // Create message
    const QString uuid = VSQUtils::createUuid();
    QString messageText = text;
    const auto attachment = m_attachmentBuilder.build(attachmentUrl.toUrl(), attachmentType);
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
