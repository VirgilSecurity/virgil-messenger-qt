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

#include "DownloadAttachmentOperation.h"
#include "Messenger.h"

#include <QPointer>
#include <QLoggingCategory>


Q_DECLARE_LOGGING_CATEGORY(lcMessagesQueue);


class QThreadPool;


class Settings;


namespace vm
{
class MessageOperation;
class MessageOperationFactory;
class UserDatabase;

class MessagesQueue : public QObject
{
    Q_OBJECT

public:
    using PostDownloadFunction = std::function<void ()>;

    struct DownloadParameter : DownloadAttachmentOperation::Parameter
    {
        PostDownloadFunction postFunction;
    };

    struct Item
    {
        ModifiableMessageHandler message;
        std::optional<DownloadParameter> download = std::nullopt; // Parameter for download/preload
        qsizetype attemptCount = 0;
    };

    MessagesQueue(const Settings *settings, Messenger *messenger, UserDatabase *userDatabase, QObject *parent);
    ~MessagesQueue() override;

signals:
    void pushMessage(const ModifiableMessageHandler &message);
    void pushMessageDownload(const ModifiableMessageHandler &message, const QString &filePath, const PostDownloadFunction &func);
    void pushMessagePreload(const ModifiableMessageHandler &message);

    void updateMessage(const MessageUpdate &messagesUpdate);
    void notificationCreated(const QString &notification, const bool error);

    void itemFailed(Item item, QPrivateSignal);
    void stopRequested(QPrivateSignal);

private:
    void run();
    void stop();
    void addItem(Item item, const bool run);
    void runItem(Item item);

    void onPushMessage(const ModifiableMessageHandler &message);
    void onPushMessageDownload(const ModifiableMessageHandler &message, const QString &filePath, const PostDownloadFunction &postFunction);
    void onPushMessagePreload(const ModifiableMessageHandler &message);

    void onDatabaseOpened();
    void onOnlineStatusChanged(const bool isOnline);
    void onNotSentMessagesFetched(const ModifiableMessages &messages);
    void onItemFailed(Item item);

    QPointer<Messenger> m_messenger;
    QPointer<UserDatabase> m_userDatabase;
    QPointer<MessageOperationFactory> m_factory;
    QPointer<QThreadPool> m_threadPool;

    std::vector<Item> m_items;
    std::atomic_bool m_isStopped = false;
};
}

Q_DECLARE_METATYPE(vm::MessagesQueue::Item);
Q_DECLARE_METATYPE(vm::MessagesQueue::PostDownloadFunction);

#endif // VS_MESSAGESQUEUE_H
