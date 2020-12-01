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

#include "qxmpp/QXmppMessage.h"

#include "VSQMessenger.h"
#include "Utils.h"
#include "database/MessagesTable.h"
#include "database/UserDatabase.h"
#include "models/FileLoader.h"
#include "operations/MessageOperation.h"
#include "operations/MessageOperationFactory.h"
#include "operations/Operation.h"

using namespace vm;

MessagesQueue::MessagesQueue(const Settings *settings, VSQMessenger *messenger, UserDatabase *userDatabase, FileLoader *fileLoader, QObject *parent)
    : NetworkOperation(parent, fileLoader, fileLoader->isServiceFound())
    , m_messenger(messenger)
    , m_userDatabase(userDatabase)
    , m_factory(new MessageOperationFactory(settings, messenger, fileLoader, this))
{
    setName(QLatin1String("MessageQueue"));
    setRepeatable(true);

    connect(this, &MessagesQueue::setUserId, this, &MessagesQueue::onSetUserId);
    connect(this, &MessagesQueue::pushMessage, this, &MessagesQueue::onPushMessage);
    connect(this, &MessagesQueue::pushMessageDownload, this, &MessagesQueue::onPushMessageDownload);
    connect(this, &MessagesQueue::pushMessagePreload, this, &MessagesQueue::onPushMessagePreload);
    connect(fileLoader, &FileLoader::serviceFound, this, &MessagesQueue::onFileLoaderServiceFound);
    connect(this, &Operation::finished, this, &MessagesQueue::onFinished);
}

MessagesQueue::~MessagesQueue()
{
}

void MessagesQueue::startIfReady()
{
    if (!hasChildren()) {
        return;
    }
    if (m_queueState & QueueState::ReadyToStart) {
        start();
    }
}

void MessagesQueue::setQueueState(const MessagesQueue::QueueState &state)
{
    m_queueState = m_queueState | state;
    if (state == QueueState::UserSet) {
        connect(m_userDatabase->messagesTable(), &MessagesTable::notSentMessagesFetched, this, &MessagesQueue::onNotSentMessagesFetched);
    }
    if (m_queueState == QueueState::FetchNeeded) {
        m_queueState = m_queueState | QueueState::FetchRequested;
        m_userDatabase->messagesTable()->fetchNotSentMessages();
    }
    else {
        startIfReady();
    }
}

void MessagesQueue::unsetQueueState(const MessagesQueue::QueueState &state)
{
    m_queueState = m_queueState & ~state;
    if (state == QueueState::UserSet) {
        m_queueState = m_queueState & ~QueueState::FetchRequested;
        dropChildren();
        qCDebug(lcModel) << "MessageQueue is cleared";
    }
}

void MessagesQueue::connectMessageOperation(MessageOperation *op)
{
    connect(op, &MessageOperation::statusChanged, this, std::bind(&MessagesQueue::onMessageOperationStatusChanged, this, op));
    connect(op, &MessageOperation::attachmentStatusChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentStatusChanged, this, op));
    connect(op, &MessageOperation::attachmentUrlChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentUrlChanged, this, op));
    connect(op, &MessageOperation::attachmentLocalPathChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentLocalPathChanged, this, op));
    connect(op, &MessageOperation::attachmentFingerprintChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentFingerprintChanged, this, op));
    connect(op, &MessageOperation::attachmentExtrasChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentExtrasChanged, this, op));
    connect(op, &MessageOperation::attachmentEncryptedSizeChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentEncryptedSizeChanged, this, op));
    connect(op, &MessageOperation::attachmentProcessedSizeChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentProcessedSizeChanged, this, op));
    connect(op, &MessageOperation::notificationCreated, this, &MessagesQueue::notificationCreated);
}

MessageOperation *MessagesQueue::pushMessageOperation(const GlobalMessage &message, bool prepend)
{
    if (message.status == Message::Status::InvalidM) {
        return nullptr;
    }
    auto op = new MessageOperation(message, m_factory, this);
    connectMessageOperation(op);
    prepend ? prependChild(op) : appendChild(op);
    return op;
}

void MessagesQueue::onSetUserId(const UserId &userId)
{
    if (m_userId == userId) {
        return;
    }
    m_userId = userId;
    userId.isEmpty() ? unsetQueueState(QueueState::UserSet) : setQueueState(QueueState::UserSet);
}

void MessagesQueue::onFileLoaderServiceFound(bool serviceFound)
{
    serviceFound ? setQueueState(QueueState::FileLoaderReady) : unsetQueueState(QueueState::FileLoaderReady);
}

void MessagesQueue::onNotSentMessagesFetched(const GlobalMessages &messages)
{
    qCDebug(lcOperation) << "Queued" << messages.size() << "unsent messages";
    for (auto &m : messages) {
        if (auto op = pushMessageOperation(m)) {
            m_factory->populateAll(op);
        }
    }
    startIfReady();
}

void MessagesQueue::onFinished()
{
    qCDebug(lcModel) << "MessageQueue is finished";
}

void MessagesQueue::onPushMessage(const GlobalMessage &message)
{
    if (auto op = pushMessageOperation(message)) {
        m_factory->populateAll(op);
        startIfReady();
    }
}

void MessagesQueue::onPushMessageDownload(const GlobalMessage &message, const QString &filePath)
{
    if (auto op = pushMessageOperation(message, true)) {
        m_factory->populateDownload(op, filePath);
        connect(op, &Operation::finished, this, std::bind(&MessagesQueue::notificationCreated, this, tr("File was downloaded"), false));
        startIfReady();
    }
}

void MessagesQueue::onPushMessagePreload(const GlobalMessage &message)
{
    if (auto op = pushMessageOperation(message, true)) {
        m_factory->populatePreload(op);
        startIfReady();
    }
}

void MessagesQueue::onMessageOperationStatusChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    emit messageStatusChanged(m.id, m.contactId, m.status);
}

void MessagesQueue::onMessageOperationAttachmentStatusChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentStatusChanged(a.id, m.contactId, a.status);
}

void MessagesQueue::onMessageOperationAttachmentUrlChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentUrlChanged(a.id, m.contactId, a.url);
}

void MessagesQueue::onMessageOperationAttachmentLocalPathChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentLocalPathChanged(a.id, m.contactId, a.localPath);
}

void MessagesQueue::onMessageOperationAttachmentFingerprintChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentFingerprintChanged(a.id, m.contactId, a.fingerprint);
}

void MessagesQueue::onMessageOperationAttachmentExtrasChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentExtrasChanged(a.id, m.contactId, a.type, a.extras);
}

void MessagesQueue::onMessageOperationAttachmentProcessedSizeChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentProcessedSizeChanged(a.id, m.contactId, a.processedSize);
}

void MessagesQueue::onMessageOperationAttachmentEncryptedSizeChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentEncryptedSizeChanged(a.id, m.contactId, a.encryptedSize);
}
