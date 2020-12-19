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

#include "Messenger.h"
#include "UserDatabase.h"
#include "operations/NetworkOperation.h"

#include <QPointer>

class Settings;

namespace vm
{
class MessageOperation;
class MessageOperationFactory;

class MessagesQueue : public NetworkOperation
{
    Q_OBJECT

public:
    MessagesQueue(const Settings *settings, Messenger *messenger, UserDatabase *userDatabase, QObject *parent);
    ~MessagesQueue() override;

signals:
    void setUserId(const UserId &userId);
    void pushMessage(const MessageHandler &message);
    void pushMessageDecrypt(const MessageHandler &message);
    void pushMessageDownloadAttachment(const MessageHandler &message, const QString &filePath);
    void pushMessageDecryptAttachment(const MessageHandler &message, const QString &filePath);
    void pushMessageDownloadTumbnail(const MessageHandler &message);
    void pushMessageDecryptTumbnail(const MessageHandler &message);

    // Message operation
    void messageChanged(const MessageId &messageId, const MessageUpdate& messagesUpdate);

    void notificationCreated(const QString &notification, const bool error);

private:
    enum QueueState
    {
        Created = 0,
        UserSet = 1 << 0,
        FileLoaderReady = 1 << 1,
        FetchNeeded = UserSet | FileLoaderReady,
        FetchRequested = 1 << 2,
        ReadyToStart = UserSet | FetchRequested
    };

    void startIfReady();
    void setQueueState(const QueueState &state);
    void unsetQueueState(const QueueState &state);

    void connectMessageOperation(MessageOperation *op);
    MessageOperation *pushMessageOperation(const Message &message, bool prepend = false);

    void onSetUserId(const UserId &userId);
    void onPushMessage(const Message &message);
    void onPushMessageDownload(const Message &message, const QString &filePath);
    void onPushMessagePreload(const Message &message);

    void onFileLoaderServiceFound(bool serviceFound);
    void onNotSentMessagesFetched(const Messages &messages);
    void onFinished();

    // Message operation
    void onMessageOperationStatusChanged(const MessageOperation *operation);
    void onMessageOperationAttachmentStatusChanged(const MessageOperation *operation);
    void onMessageOperationAttachmentUrlChanged(const MessageOperation *operation);
    void onMessageOperationAttachmentLocalPathChanged(const MessageOperation *operation);
    void onMessageOperationAttachmentFingerprintChanged(const MessageOperation *operation);
    void onMessageOperationAttachmentExtrasChanged(const MessageOperation *operation);
    void onMessageOperationAttachmentProcessedSizeChanged(const MessageOperation *operation);
    void onMessageOperationAttachmentEncryptedSizeChanged(const MessageOperation *operation);

    QPointer<Messenger> m_messenger;
    QPointer<UserDatabase> m_userDatabase;
    QPointer<MessageOperationFactory> m_factory;
    UserId m_userId;
    int m_queueState = QueueState::Created;
};
}

#endif // VS_MESSAGESQUEUE_H
