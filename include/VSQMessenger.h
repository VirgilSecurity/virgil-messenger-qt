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

#ifndef VSQ_MESSENGER_H
#define VSQ_MESSENGER_H

#include <QObject>

#include "VSQCommon.h"
#include "client/VSQClient.h"
#include "models/VSQAttachmentBuilder.h"
#include "models/VSQChatsModel.h"
#include "models/VSQMessagesModel.h"

class QThread;

class VSQClient;
class VSQCrashReporter;
class VSQDatabase;
class VSQSettings;

class VSQMessenger : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString user MEMBER m_user NOTIFY userChanged)
    Q_PROPERTY(QString recipient MEMBER m_recipient NOTIFY recipientChanged)
    Q_PROPERTY(VSQMessagesModel *messages READ messageModel CONSTANT)
    Q_PROPERTY(VSQChatsModel *chats READ chatsModel CONSTANT)

public:
    explicit VSQMessenger(VSQSettings *settings, VSQCrashReporter *crashReporter, QObject *parent = nullptr);
    ~VSQMessenger() override;

    Q_INVOKABLE void start();

signals:
    void signIn(const QString &userWithEnv);
    void signInWithKey(const QString &userWithEnv, const QString &password);
    void signedIn(const QString &userWithEnv);
    void signInFailed(const QString &userWithEnv, const QString &error);

    void signOut();
    void signedOut();

    void signUp(const QString &userWithEnv);
    void signedUp(const QString &userWithEnv);
    void signUpFailed(const QString &userWithEnv, const QString &error);

    void backupKey(const QString &password);
    void keyBackuped(const QString &password);
    void backupKeyFailed(const QString &password, const QString &error);

    void addContact(const QString &contact);
    void contactAdded(const QString &contact);
    void addContactFailed(const QString &contact, const QString &error);

    void createSendMessage(const QString &text, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType);
    void sendMessage(const Message &message);
    void messageSent();
    void sendMessageFailed();

    void quitRequested();
    void credentialsRequested(bool signOut);
    void checkConnectionState();
    void setOnlineStatus(bool online);

    void userChanged(const QString &user);
    void recipientChanged(const QString &recipient);

private:
    void setupConnections();

    void setUser(const QString &user);
    void setRecipient(const QString &recipient);

    VSQMessagesModel *messageModel();
    VSQChatsModel *chatsModel();

    void onCreateSendMessage(const QString &text, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType);

    VSQSettings *m_settings;
    VSQCrashReporter *m_crashReporter;
    VSQAttachmentBuilder m_attachmentBuilder;
    VSQMessagesModel m_messageModel;
    VSQChatsModel m_chatsModel;

    VSQDatabase *m_database;
    QThread *m_databaseThread;
    VSQClient *m_client;
    QThread *m_clientThread;

    QString m_user;
    QString m_recipient;
};

#endif // VSQ_MESSENGER_H
