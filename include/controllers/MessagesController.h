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

#ifndef VM_MESSAGESCONTROLLER_H
#define VM_MESSAGESCONTROLLER_H

#include "Messenger.h"
#include "Models.h"
#include "UserDatabase.h"

#include <QObject>


namespace vm
{

class MessagesController : public QObject
{
    Q_OBJECT

public:
    MessagesController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent);

    void loadMessages(const ChatHandler &chat);
    void clearMessages();
    Q_INVOKABLE void sendTextMessage(const QString &body);
    Q_INVOKABLE void sendFileMessage(const QVariant &attachmentUrl);
    Q_INVOKABLE void sendPictureMessage(const QVariant &attachmentUrl);

signals:
    void errorOccurred(const QString &errorText);
    void notificationCreated(const QString &notification, const bool error);

    void messageCreated(const MessageHandler &message);
    void messageUpdated(const MessageUpdate &messageUpdate);

    void displayImageNotFound(const MessageId &messageId);

private:
    void setupTableConnections();

    void updateMessage(const MessageId &messageId, const MessageUpdate &messageUpdate);

private slots:
    void onChatUpdated(const Chat &chat);
    void onMessageReceived(const MessageHandler &message);
    void onMessageUpdated(const MessageId &messageId, const MessageUpdate& messageUpdate);

private:
    QPointer<Messenger> m_messenger;
    QPointer<Models> m_models;
    QPointer<UserDatabase> m_userDatabase;

    ChatHandler m_chat;
};
} // namespace vm

#endif // VM_MESSAGESCONTROLLER_H
