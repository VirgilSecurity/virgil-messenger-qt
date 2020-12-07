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

#include <QtConcurrent>
#include <QThreadPool>

#include "database/MessagesTable.h"
#include "database/UserDatabase.h"
#include "models/FileLoader.h"
#include "operations/MessageOperation.h"
#include "operations/MessageOperationFactory.h"

using namespace vm;

MessagesQueue::MessagesQueue(const Settings *settings, VSQMessenger *messenger, UserDatabase *userDatabase, FileLoader *fileLoader, QObject *parent)
    : QObject(parent)
    , m_fileLoader(fileLoader)
    , m_threadPool(new QThreadPool(this))
    , m_userDatabase(userDatabase)
    , m_factory(new MessageOperationFactory(settings, messenger, fileLoader, this))
    , m_isStopped(false)
{
    qRegisterMetaType<MessagesQueue::OperationItem>("OperationItem");

    m_threadPool->setMaxThreadCount(5);
    connect(fileLoader, &FileLoader::serviceFound, this, &MessagesQueue::onFileLoaderServiceFound);
    connect(this, &MessagesQueue::operationFailed, this, std::bind(&MessagesQueue::addOperationItem, this, args::_1, false));
}

MessagesQueue::~MessagesQueue()
{
    cleanup();
    m_threadPool->deleteLater();
}

void MessagesQueue::setUserId(const UserId &userId)
{
    if (m_userId == userId) {
        return;
    }
    m_userId = userId;
    userId.isEmpty() ? unsetQueueState(QueueState::UserSet) : setQueueState(QueueState::UserSet);
}

void MessagesQueue::pushMessage(const GlobalMessage &message)
{
    addOperationItem({ message, [=](MessageOperation *op) {
        m_factory->populateAll(op);
    }});
}

void MessagesQueue::pushMessageDownload(const GlobalMessage &message, const QString &filePath)
{
    addOperationItem({ message, [=](MessageOperation *op) {
        m_factory->populateDownload(op, filePath);
        connect(op, &Operation::finished, this, std::bind(&MessagesQueue::notificationCreated, this, tr("File was downloaded"), false));
    }});
}

void MessagesQueue::pushMessagePreload(const GlobalMessage &message)
{
    addOperationItem({ message, [=](MessageOperation *op) {
        m_factory->populatePreload(op);
    }});
}

void MessagesQueue::setQueueState(const MessagesQueue::QueueState &state)
{
    m_queueState = m_queueState | state;
    if (state == QueueState::UserSet) {
        connect(m_userDatabase->messagesTable(), &MessagesTable::notSentMessagesFetched, this, &MessagesQueue::onNotSentMessagesFetched);
        m_isStopped = false;
    }
    if (m_queueState == QueueState::FetchNeeded) {
        m_queueState = m_queueState | QueueState::FetchRequested;
        m_userDatabase->messagesTable()->fetchNotSentMessages();
    }
    else {
        runIfReady();
    }
}

void MessagesQueue::unsetQueueState(const MessagesQueue::QueueState &state)
{
    m_queueState = m_queueState & ~state;
    if (state == QueueState::UserSet) {
        m_queueState = m_queueState & ~QueueState::FetchRequested;
        cleanup();
    }
}

void MessagesQueue::runIfReady()
{
    if ((m_queueState & QueueState::ReadyToRun) != QueueState::ReadyToRun) {
        return;
    }
    OperationItems items;
    std::swap(items, m_items);
    for (auto &item : items) {
        runOperation(item);
    }
}

void MessagesQueue::addOperationItem(const OperationItem &item, bool run)
{
    if (item.message.status == Message::Status::InvalidM) {
        return;
    }
    m_items.push_back(item);
    if (run) {
        this->runIfReady();
    }
}

void MessagesQueue::runOperation(const OperationItem &item)
{
    QtConcurrent::run(m_threadPool, [=, &stopped = m_isStopped]() {
        if (stopped) {
            qCDebug(lcModel) << "Queue was stopped. Operation is skipped";
            return;
        }
        auto op = new MessageOperation(item.message, m_factory, m_fileLoader, nullptr);
        // connect
        connect(op, &MessageOperation::statusChanged, this, &MessagesQueue::messageStatusChanged);
        connect(op, &MessageOperation::attachmentStatusChanged, this, &MessagesQueue::attachmentStatusChanged);
        connect(op, &MessageOperation::attachmentUrlChanged, this, &MessagesQueue::attachmentUrlChanged);
        connect(op, &MessageOperation::attachmentLocalPathChanged, this, &MessagesQueue::attachmentLocalPathChanged);
        connect(op, &MessageOperation::attachmentFingerprintChanged, this, &MessagesQueue::attachmentFingerprintChanged);
        connect(op, &MessageOperation::attachmentExtrasChanged, this, &MessagesQueue::attachmentExtrasChanged);
        connect(op, &MessageOperation::attachmentEncryptedSizeChanged, this, &MessagesQueue::attachmentEncryptedSizeChanged);
        connect(op, &MessageOperation::attachmentProcessedSizeChanged, this, &MessagesQueue::attachmentProcessedSizeChanged);
        connect(op, &MessageOperation::notificationCreated, this, &MessagesQueue::notificationCreated);
        connect(this, &MessagesQueue::stopRequested, op, &MessageOperation::stop);
        // setup
        item.setup(op);
        // start & wait for done
        op->start();
        op->waitForDone();
        // notify MessagesQueue about failed operation
        if (op->status() == Operation::Status::Failed) {
            emit operationFailed({ *op->message(), item.setup }, QPrivateSignal());
        }
        op->drop(true);
    });
}

void MessagesQueue::cleanup()
{
    qCDebug(lcModel) << "Cleanup message queue";
    m_items.clear();
    m_isStopped = true;
    emit stopRequested(QPrivateSignal());
    m_threadPool->waitForDone();
}

void MessagesQueue::onFileLoaderServiceFound(const bool serviceFound)
{
    serviceFound ? setQueueState(QueueState::FileLoaderReady) : unsetQueueState(QueueState::FileLoaderReady);
}

void MessagesQueue::onNotSentMessagesFetched(const GlobalMessages &messages)
{
    qCDebug(lcOperation) << "Queued" << messages.size() << "unsent messages";
    for (auto &m : messages) {
        pushMessage(m);
    }
}
