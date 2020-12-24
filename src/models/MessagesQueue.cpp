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

    connect(m_userDatabase, &UserDatabase::opened, this, &Self::onDatabaseOpened);
    connect(this, &Self::pushMessage, this, &Self::onPushMessage);
    connect(this, &Self::pushMessageDownload, this, &Self::onPushMessageDownload);
    connect(this, &Self::pushMessagePreload, this, &Self::onPushMessagePreload);
}


Self::~MessagesQueue() {
    stop();
    m_threadPool->deleteLater(); // TODO(fpohtmeh): remove?
}


void Self::runOperation(MessageHandler message, const OperationType operationType, const QVariant &data)
{
    QtConcurrent::run(m_threadPool, [=]() {
        if (m_isStopped) {
            qCDebug(lcMessagesQueue) << "Operation was skipped because queue was stopped";
            return;
        }
        auto op = new MessageOperation(message, m_factory, m_messenger->isOnline(), nullptr);
        connect(op, &MessageOperation::messageUpdate, this, &Self::updateMessage);
        connect(m_messenger, &Messenger::onlineStatusChanged, op, &NetworkOperation::setIsOnline);
        connect(op, &MessageOperation::notificationCreated, this, &MessagesQueue::notificationCreated);
        connect(this, &MessagesQueue::stopRequested, op, &MessageOperation::stop);
        switch (operationType) {
            case OperationType::Download:
                m_factory->populateDownload(op, data.toString());
                connect(op, &Operation::finished,
                        this, std::bind(&Self::notificationCreated, this, tr("File was downloaded"), false));
                break;
            case OperationType::Preload:
                m_factory->populatePreload(op);
                break;
            case OperationType::Full:
            default:
                m_factory->populateAll(op);
                break;
        }
        op->start();
        op->waitForDone();
        // TODO(fpohtmeh): add retry
        op->drop(true);
    });
}


void Self::stop()
{
    qCDebug(lcMessagesQueue) << "Cleanup messages queue";
    m_isStopped = true; // TODO(fpohtmeh): set false on user set
    emit stopRequested(QPrivateSignal());
    m_threadPool->waitForDone();
}


void Self::onDatabaseOpened() {
    connect(m_userDatabase->messagesTable(), &MessagesTable::notSentMessagesFetched, this, &Self::onNotSentMessagesFetched);
    m_userDatabase->messagesTable()->fetchNotSentMessages();
}


void Self::onNotSentMessagesFetched(const ModifiableMessages &messages)
{
    qCDebug(lcMessagesQueue) << "Queued" << messages.size() << "unsent messages";
    for (auto &m : messages) {
        onPushMessage(m);
    }
}


void Self::onPushMessage(const MessageHandler &message)
{
    runOperation(message, OperationType::Full, QVariant());
}


void Self::onPushMessageDownload(const MessageHandler &message, const QString &filePath)
{
    runOperation(message, OperationType::Download, filePath);
}


void Self::onPushMessagePreload(const MessageHandler &message)
{
    runOperation(message, OperationType::Preload, QVariant());
}
