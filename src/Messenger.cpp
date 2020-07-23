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

#include "Client.h"
#include "Database.h"
#include "Settings.h"
#include "Utils.h"
#include "VirgilCore.h"

Messenger::Messenger(Settings *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
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
    // Authorization calls
    connect(this, &Messenger::signIn, m_client, &Client::signIn);
    connect(this, &Messenger::signOut, m_client, &Client::signOut);
    connect(this, &Messenger::signUp, m_client, &Client::signUp);
    connect(this, &Messenger::backupKey, m_client, &Client::backupKey);
    connect(this, &Messenger::signInWithKey, m_client, &Client::signInWithKey);

    // Authorization results processing/redirection
    connect(m_client, &Client::signInSuccess, this, &Messenger::signInSuccess);
    connect(m_client, &Client::signInSuccess, m_settings, &Settings::addUserToList);
    connect(m_client, &Client::signInSuccess, m_settings, &Settings::setLastSignedInUser);
    connect(m_client, &Client::signInSuccess, this, &Messenger::setUser);
    connect(m_client, &Client::signInError, this, &Messenger::signInError);
    connect(m_client, &Client::signOutSuccess, this, std::bind(&Messenger::credentialsRequested, this, true));
    connect(m_client, &Client::signOutSuccess, this, std::bind(&Messenger::setUser, this, QLatin1String()));
    connect(m_client, &Client::signOutSuccess, this, &Messenger::signOutSuccess);
    connect(m_client, &Client::signUpSuccess, this, &Messenger::signUpSuccess);
    connect(m_client, &Client::signUpSuccess, m_settings, &Settings::addUserToList);
    connect(m_client, &Client::signUpSuccess, m_settings, &Settings::setLastSignedInUser);
    connect(m_client, &Client::signUpError, this, &Messenger::signUpError);
    connect(m_client, &Client::backupKeySuccess, this, &Messenger::backupKeySuccess);
    connect(m_client, &Client::backupKeyError, this, &Messenger::backupKeyError);

    // Contacts & messages
    connect(this, &Messenger::addContact, m_client, &Client::addContact);
    connect(this, &Messenger::createSendMessage, this, &Messenger::onCreateSendMessage);
    connect(this, &Messenger::sendMessage, m_client, &Client::sendMessage);
    connect(m_client, &Client::addContactSuccess, this, &Messenger::addContactSuccess);
    connect(m_client, &Client::addContactError, this, &Messenger::addContactError);
    connect(m_client, &Client::sendMessageSuccess, this, &Messenger::sendMessageSuccess);
    connect(m_client, &Client::sendMessageError, this, &Messenger::sendMessageError);

    // Other calls
    connect(this, &Messenger::checkConnectionState, m_client, &Client::checkConnectionState);
    connect(this, &Messenger::setOnlineStatus, m_client, &Client::setOnlineStatus);

    // Sign-in or request credentials
    if (m_settings->lastSignedInUser().isEmpty())
        emit credentialsRequested(false);
    else
        signIn(m_settings->lastSignedInUser());
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

void Messenger::onCreateSendMessage(const QString &text, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType)
{
    // TODO(fpohtmeh): create from atttachment
    (void) attachmentUrl;
    (void) attachmentType;
    Optional<Attachment> attachment;
    // Create message
    const QString uuid = Utils::createUuid();
    QString messageText = text;
    if (attachment)
        messageText = attachment->fileName();
    const StMessage stMessage{ uuid, messageText, StMessage::Author::User, m_recipient, attachment };
    // Emit
    emit sendMessage(stMessage);
}
