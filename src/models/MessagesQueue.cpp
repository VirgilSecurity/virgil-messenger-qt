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

#include "models/MessagesQueue.h"


#include "Messenger.h"
#include "Utils.h"
#include "MessagesTable.h"
#include "UserDatabase.h"
#include "MessageOperation.h"
#include "MessageOperationFactory.h"


#include <QtConcurrent>


using namespace vm;
using Self = MessagesQueue;


Q_LOGGING_CATEGORY(lcMessagesQueue, "messages-queue");


Self::MessagesQueue(const Settings *settings, Messenger *messenger, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
    , m_userDatabase(userDatabase)
    , m_factory(new MessageOperationFactory(settings, messenger, this))
    , m_threadPool(new QThreadPool(this))
{
    m_threadPool->setMaxThreadCount(5);

    qRegisterMetaType<vm::MessagesQueue::Item>("Item");

    connect(m_messenger, &Messenger::onlineStatusChanged, this, &MessagesQueue::onOnlineStatusChanged);
    connect(m_messenger, &Messenger::signedOut, this, &MessagesQueue::stop);
    connect(m_userDatabase, &UserDatabase::opened, this, &Self::onDatabaseOpened);
    connect(this, &Self::pushMessage, this, &Self::onPushMessage);
    connect(this, &Self::pushMessageDownload, this, &Self::onPushMessageDownload);
    connect(this, &Self::pushMessagePreload, this, &Self::onPushMessagePreload);
    connect(this, &Self::itemFailed, this, &Self::onItemFailed);
}


Self::~MessagesQueue()
{
    stop();
}


void Self::run()
{
    std::vector<Item> items;
    std::swap(items, m_items);
    for (auto &item : items) {
        runItem(item);
    }
}


void Self::stop()
{
    qCDebug(lcMessagesQueue) << "stop";
    m_items.clear();
    m_isStopped = true;
    emit stopRequested(QPrivateSignal());
    m_threadPool->waitForDone();
}


void Self::addItem(Item item, const bool run)
{
    if (item.message->status() == MessageStatus::Broken) {
        // Broken message can't be processed (downloaded, preloaded, etc)
        return;
    }
    m_items.push_back(std::move(item));
    if (run) {
        this->run();
    }
}


void Self::runItem(Item item)
{
    QtConcurrent::run(m_threadPool, [=, item = std::move(item)]() {
        if (m_isStopped) {
            qCDebug(lcMessagesQueue) << "Operation was skipped because queue was stopped";
            return;
        }
        auto op = new MessageOperation(item.message, m_factory, m_messenger->isOnline(), nullptr);
        connect(op, &MessageOperation::messageUpdate, this, &Self::updateMessage);
        connect(m_messenger, &Messenger::onlineStatusChanged, op, &NetworkOperation::setIsOnline);
        connect(op, &MessageOperation::notificationCreated, this, &MessagesQueue::notificationCreated);
        connect(this, &MessagesQueue::stopRequested, op, &MessageOperation::stop);
        switch (item.actionType) {
            case Item::ActionType::Download:
                m_factory->populateDownload(op, item.parameter.toString());
                connect(op, &Operation::finished, this, std::bind(&Self::notificationCreated, this, tr("File was downloaded"), false));
                break;
            case Item::ActionType::Preload:
                m_factory->populatePreload(op);
                break;
            case Item::ActionType::SendReceive:
            default:
                m_factory->populateAll(op);
                break;
        }
        op->start();
        op->waitForDone();
        if (op->status() == Operation::Status::Failed) {
            emit itemFailed(item, QPrivateSignal());
        }
        op->drop(true);
    });
}


void Self::onDatabaseOpened()
{
    m_isStopped = false;
    connect(m_userDatabase->messagesTable(), &MessagesTable::notSentMessagesFetched, this, &Self::onNotSentMessagesFetched);
    m_userDatabase->messagesTable()->fetchNotSentMessages();
}


void MessagesQueue::onOnlineStatusChanged(const bool isOnline)
{
    if (isOnline) {
        run();
    }
}


void Self::onNotSentMessagesFetched(const ModifiableMessages &messages)
{
    qCDebug(lcMessagesQueue) << "Queued" << messages.size() << "unsent messages";
    for (auto &m : messages) {
        pushMessage(m);
    }
}


void MessagesQueue::onItemFailed(Item item)
{
    const int maxAttemptCount = 3;
    const auto &message = item.message;
    if (item.attemptCount < maxAttemptCount) {
        qCDebug(lcMessagesQueue) << "Enqueued failed message:" << message->id();
        ++item.attemptCount;
        addItem(std::move(item), false);
    }
    else if (item.attemptCount == maxAttemptCount) {
        // Mark as broken
        if (message->isIncoming()) {
            IncomingMessageStageUpdate update;
            update.messageId = message->id();
            update.stage = IncomingMessageStage::Broken;
            emit updateMessage(update);
            qCDebug(lcMessagesQueue) << "Incoming message was marked as broken:" << message->id();
        }
        else if (message->isOutgoing()) {
            OutgoingMessageStageUpdate update;
            update.messageId = message->id();
            update.stage = OutgoingMessageStage::Broken;
            emit updateMessage(update);
            qCDebug(lcMessagesQueue) << "Outgoing message was marked as broken:" << message->id();
        }
    }
}


void Self::onPushMessage(const MessageHandler &message)
{
    if (message->isIncoming() || message->isOutgoingCopyFromOtherDevice()) {
        pushMessagePreload(message);
    }
    else {
        addItem({ message }, true);
    }
}


void Self::onPushMessageDownload(const MessageHandler &message, const QString &filePath)
{
    addItem({ message, Item::ActionType::Download, filePath }, true);
}


void Self::onPushMessagePreload(const MessageHandler &message)
{
    addItem({ message, Item::ActionType::Preload }, true);
}
