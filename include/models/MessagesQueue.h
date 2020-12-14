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

#include "VSQCommon.h"
#include "operations/Operation.h"

class QThreadPool;

class VSQMessenger;
class Settings;

namespace vm
{
class FileLoader;
class MessageOperation;
class MessageOperationFactory;
class UserDatabase;

class MessagesQueue : public QObject
{
    Q_OBJECT

public:
    struct OperationItem
    {
        GlobalMessage message;
        std::function<void (MessageOperation *)> setup;
    };
    using OperationItems = std::vector<OperationItem>;

    MessagesQueue(const Settings *settings, VSQMessenger *messenger, UserDatabase *userDatabase, FileLoader *fileLoader, QObject *parent);
    ~MessagesQueue() override;

    void setUserId(const UserId &userId);
    void pushMessage(const GlobalMessage &message);
    void pushMessageDownload(const GlobalMessage &message, const QString &filePath);
    void pushMessagePreload(const GlobalMessage &message);

signals:
    void messageStatusChanged(const Message::Id &messageId, const Contact::Id &contactId, const Message::Status status);
    void attachmentStatusChanged(const Attachment::Id &attachmentId, const Contact::Id &contactId, const Attachment::Status &status);
    void attachmentUrlChanged(const Attachment::Id &attachmentId, const Contact::Id &contactId, const QUrl &url);
    void attachmentLocalPathChanged(const Attachment::Id &attachmentId, const Contact::Id &contactId, const QString &localPath);
    void attachmentFingerprintChanged(const Attachment::Id &attachmentId, const Contact::Id &contactId, const QString &fingerprint);
    void attachmentExtrasChanged(const Attachment::Id &attachmentId, const Contact::Id &contactId, const Attachment::Type &type, const QVariant &extras);
    void attachmentProcessedSizeChanged(const Attachment::Id &attachmentId, const Contact::Id &contactId, const DataSize &processedSize);
    void attachmentEncryptedSizeChanged(const Attachment::Id &attachmentId, const Contact::Id &contactId, const DataSize &encryptedSize);

    void notificationCreated(const QString &notification, const bool error);

    void stopRequested(QPrivateSignal);
    void operationFailed(const OperationItem &item, QPrivateSignal);

private:
    enum QueueState : Flag
    {
        Created = 0,
        UserSet = 1 << 0,
        FileLoaderReady = 1 << 1,
        FetchNeeded = UserSet | FileLoaderReady,
        FetchRequested = 1 << 2,
        ReadyToRun = UserSet | FetchRequested
    };

    void setQueueState(const QueueState &state);
    void unsetQueueState(const QueueState &state);

    void runIfReady();
    void addOperationItem(const OperationItem &item, bool run = true);
    void runOperation(const OperationItem &item);
    void cleanup();

    void onFileLoaderServiceFound(const bool serviceFound);
    void onNotSentMessagesFetched(const GlobalMessages &messages);

    FileLoader *m_fileLoader;
    QThreadPool *m_threadPool;
    UserDatabase *m_userDatabase;
    MessageOperationFactory *m_factory;
    UserId m_userId;

    Flag m_queueState = QueueState::Created;
    OperationItems m_items;
    std::atomic_bool m_isStopped;
};
}

Q_DECLARE_METATYPE(vm::MessagesQueue::OperationItem)

#endif // VS_MESSAGESQUEUE_H
