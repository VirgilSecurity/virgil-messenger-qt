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

#include "Messenger.h"
#include "Utils.h"
#include "MessagesTable.h"
#include "UserDatabase.h"
#include "FileLoader.h"
#include "MessageOperation.h"
#include "MessageOperationFactory.h"
#include "Operation.h"

using namespace vm;
using Self = MessagesQueue;

Self::MessagesQueue(const Settings *settings, Messenger *messenger, UserDatabase *userDatabase, QObject *parent)
    : NetworkOperation(parent, messenger->isOnline())
    , m_messenger(messenger)
    , m_userDatabase(userDatabase)
    , m_factory(new MessageOperationFactory(settings, messenger, this))
{
    setName(QLatin1String("MessageQueue"));
    setRepeatable(true);

    connect(messenger, &Messenger::onlineStatusChanged, this, &NetworkOperation::setIsOnline);
    connect(this, &Self::setUserId, this, &Self::onSetUserId);
    connect(this, &Self::pushMessage, this, &Self::onPushMessage);
    connect(this, &Self::pushMessageDownload, this, &Self::onPushMessageDownload);
    connect(this, &Self::pushMessagePreload, this, &Self::onPushMessagePreload);
    connect(messenger->fileLoader(), &FileLoader::uploadServiceFound, this, &Self::onFileLoaderServiceFound);
    connect(this, &Operation::finished, this, &Self::onFinished);
}

Self::~MessagesQueue()
{
}

void Self::startIfReady()
{
    if (!hasChildren()) {
        return;
    }
    if (m_queueState & QueueState::ReadyToStart) {
        start();
    }
}

void Self::setQueueState(const Self::QueueState &state)
{
    m_queueState = m_queueState | state;
    if (state == QueueState::UserSet) {
        connect(m_userDatabase->messagesTable(), &MessagesTable::notSentMessagesFetched, this, &Self::onNotSentMessagesFetched);
    }
    if (m_queueState == QueueState::FetchNeeded) {
        m_queueState = m_queueState | QueueState::FetchRequested;
        m_userDatabase->messagesTable()->fetchNotSentMessages();
    }
    else {
        startIfReady();
    }
}

void Self::unsetQueueState(const Self::QueueState &state)
{
    m_queueState = m_queueState & ~state;
    if (state == QueueState::UserSet) {
        m_queueState = m_queueState & ~QueueState::FetchRequested;
        dropChildren();
        qCDebug(lcModel) << "MessageQueue is cleared";
    }
}

void Self::connectMessageOperation(MessageOperation *op)
{
    connect(op, &MessageOperation::statusChanged, this, std::bind(&Self::onMessageOperationStatusChanged, this, op));
    connect(op, &MessageOperation::attachmentStatusChanged, this, std::bind(&Self::onMessageOperationAttachmentStatusChanged, this, op));
    connect(op, &MessageOperation::attachmentUrlChanged, this, std::bind(&Self::onMessageOperationAttachmentUrlChanged, this, op));
    connect(op, &MessageOperation::attachmentLocalPathChanged, this, std::bind(&Self::onMessageOperationAttachmentLocalPathChanged, this, op));
    connect(op, &MessageOperation::attachmentFingerprintChanged, this, std::bind(&Self::onMessageOperationAttachmentFingerprintChanged, this, op));
    connect(op, &MessageOperation::attachmentExtrasChanged, this, std::bind(&Self::onMessageOperationAttachmentExtrasChanged, this, op));
    connect(op, &MessageOperation::attachmentEncryptedSizeChanged, this, std::bind(&Self::onMessageOperationAttachmentEncryptedSizeChanged, this, op));
    connect(op, &MessageOperation::attachmentProcessedSizeChanged, this, std::bind(&Self::onMessageOperationAttachmentProcessedSizeChanged, this, op));
    connect(op, &MessageOperation::notificationCreated, this, &Self::notificationCreated);
}

MessageOperation *Self::pushMessageOperation(const GlobalMessage &message, bool prepend)
{
    if (message.status == Message::Status::InvalidM) {
        return nullptr;
    }
    auto op = new MessageOperation(message, m_factory, this);
    connectMessageOperation(op);
    prepend ? prependChild(op) : appendChild(op);
    return op;
}

void Self::onSetUserId(const UserId &userId)
{
    if (m_userId == userId) {
        return;
    }
    m_userId = userId;
    userId.isEmpty() ? unsetQueueState(QueueState::UserSet) : setQueueState(QueueState::UserSet);
}

void Self::onFileLoaderServiceFound(bool serviceFound)
{
    serviceFound ? setQueueState(QueueState::FileLoaderReady) : unsetQueueState(QueueState::FileLoaderReady);
}

void Self::onNotSentMessagesFetched(const GlobalMessages &messages)
{
    qCDebug(lcOperation) << "Queued" << messages.size() << "unsent messages";
    for (auto &m : messages) {
        if (auto op = pushMessageOperation(m)) {
            m_factory->populateAll(op);
        }
    }
    startIfReady();
}

void Self::onFinished()
{
    qCDebug(lcModel) << "MessageQueue is finished";
}

void Self::onPushMessage(const GlobalMessage &message)
{
    if (auto op = pushMessageOperation(message)) {
        m_factory->populateAll(op);
        startIfReady();
    }
}

void Self::onPushMessageDownload(const GlobalMessage &message, const QString &filePath)
{
    if (auto op = pushMessageOperation(message, true)) {
        m_factory->populateDownload(op, filePath);
        connect(op, &Operation::finished, this, std::bind(&Self::notificationCreated, this, tr("File was downloaded"), false));
        startIfReady();
    }
}

void Self::onPushMessagePreload(const GlobalMessage &message)
{
    if (auto op = pushMessageOperation(message, true)) {
        m_factory->populatePreload(op);
        startIfReady();
    }
}

void Self::onMessageOperationStatusChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    emit messageStatusChanged(m.id, m.contactId, m.status);
}

void Self::onMessageOperationAttachmentStatusChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentStatusChanged(a.id, m.contactId, a.status);
}

void Self::onMessageOperationAttachmentUrlChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentUrlChanged(a.id, m.contactId, a.url);
}

void Self::onMessageOperationAttachmentLocalPathChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentLocalPathChanged(a.id, m.contactId, a.localPath);
}

void Self::onMessageOperationAttachmentFingerprintChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentFingerprintChanged(a.id, m.contactId, a.fingerprint);
}

void Self::onMessageOperationAttachmentExtrasChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentExtrasChanged(a.id, m.contactId, a.type, a.extras);
}

void Self::onMessageOperationAttachmentProcessedSizeChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentProcessedSizeChanged(a.id, m.contactId, a.processedSize);
}

void Self::onMessageOperationAttachmentEncryptedSizeChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentEncryptedSizeChanged(a.id, m.contactId, a.encryptedSize);
}
