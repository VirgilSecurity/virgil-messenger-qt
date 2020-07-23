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

#ifndef VSQ_CLIENT_H
#define VSQ_CLIENT_H

#include <QObject>

#include <QXmppClient.h>

#include "VirgilCore.h"

class Client : public QObject
{
    Q_OBJECT

public:
    Client(Settings *settings, QObject *parent);

    void start();

signals:
    void signIn(const QString &userWithEnv);
    void signOut();
    void signUp(const QString &user);
    void backupKey(const QString &password);
    void signInWithKey(const QString &userWithEnv, const QString &password);

    void signInSuccess(const QString &userWithEnv);
    void signInError(const QString &userWithEnv, const QString &error);
    void signOutSuccess();
    void signUpSuccess(const QString &userWithEnv);
    void signUpError(const QString &userWithEnv, const QString &error);
    void backupKeySuccess(const QString &password);
    void backupKeyError(const QString &password, const QString &error);

    void addContact(const QString &contact);
    void sendMessage(const StMessage &message);
    void checkConnectionState();
    void setOnlineStatus(bool online);

    void addContactSuccess(const QString &contact);
    void addContactError(const QString &contact, const QString &error);
    void sendMessageSuccess(const StMessage &message);
    void sendMessageError(const StMessage &message, const QString &error);
    void receiveMessageError(const QString &error);

private:
    bool xmppConnect();
    void waitForConnectionChange();
    void xmppDisconnect();
    void xmppReconnect();
    void subscribeOnPushNotifications(bool enable);

    void onSignIn(const QString &userWithEnv);
    void onSignOut();
    void onSignUp(const QString &userWithEnv);
    void onBackupKey(const QString &password);
    void onSignInWithKey(const QString &user, const QString &password);
    void onAddContact(const QString &contact);

    void onConnected();
    void onDisconnected();
    void onError(QXmppClient::Error error);
    void onMessageReceived(const QXmppMessage &message);
    void onPresenceReceived(const QXmppPresence &presence);
    void onIqReceived(const QXmppIq &iq);
    void onStateChanged(QXmppClient::State state);

    void onSendMessage(const StMessage &message);
    void onCheckConnectionState();
    void onSetOnlineStatus(bool online);
    void onMessageDelivered(const QString &jid, const QString &id);
    void onXmppLoggerMessage(QXmppLogger::MessageType type, const QString &message);
    void onSslErrors(const QList<QSslError> &errors);

    QString m_lastErrorText;
    VirgilCore m_core;
    QXmppClient m_client;
};

#endif // VSQ_CLIENT_H
