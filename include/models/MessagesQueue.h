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

#ifndef VS_MESSAGESQUEUE_H
#define VS_MESSAGESQUEUE_H

#include <queue>

#include "VSQCommon.h"

class VSQMessenger;

namespace vm
{
class UserDatabase;

class MessagesQueue : public QObject
{
    Q_OBJECT

public:
    MessagesQueue(VSQMessenger *messenger, UserDatabase *userDatabase, QObject *parent);

signals:
    void pushMessage(const Message &message, const Contact::Id &sender, const Contact::Id &recipient);

    void sendFailedMessages();
    void sendNextMessage(QPrivateSignal);

    void messageStatusChanged(const Message::Id &messageId, const Contact::Id &contactId, const Message::Status status);

private:
    void setUserId(const UserId &userId);
    void setFailedMessages(const QueueMessages &messages);

    void onPushMessage(const Message &message, const Contact::Id &sender, const Contact::Id &recipient);

    void onSendFailedMessages();
    void onSendNextMessage();

    void onMessageStatusChanged(const Message::Id &messageId, const Contact::Id &contactId, const Message::Status status);

    Message::Status sendMessage(const QueueMessage &message);

    VSQMessenger *m_messenger;
    UserDatabase *m_userDatabase;
    std::queue<QueueMessage> m_messages;
    UserId m_userId;
};
}

#endif // VS_MESSAGESQUEUE_H
