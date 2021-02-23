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

using namespace vm;
using Self = MessagesQueue;
using DownloadParameter = MessageOperationSource::DownloadParameter;

Q_LOGGING_CATEGORY(lcMessagesQueue, "messages-queue");

Self::MessagesQueue(Messenger *messenger, UserDatabase *userDatabase, QObject *parent)
    : OperationQueue(lcMessagesQueue(), parent),
      m_messenger(messenger),
      m_userDatabase(userDatabase),
      m_factory(new MessageOperationFactory(messenger, this))
{
    connect(m_messenger, &Messenger::onlineStatusChanged, this, &MessagesQueue::onOnlineStatusChanged);
    connect(m_messenger, &Messenger::signedOut, this, &MessagesQueue::stop);
    connect(m_userDatabase, &UserDatabase::opened, this, &Self::onDatabaseOpened);
    connect(this, &Self::pushMessage, this, &Self::onPushMessage);
    connect(this, &Self::pushMessageDownload, this, &Self::onPushMessageDownload);
    connect(this, &Self::pushMessagePreload, this, &Self::onPushMessagePreload);
}

Self::~MessagesQueue() { }

Operation *Self::createOperation(OperationSourcePtr source)
{
    const auto messageSource = dynamic_cast<MessageOperationSource *>(source.get());

    auto op = new MessageOperation(messageSource->message(), m_factory, m_messenger->isOnline(), nullptr);
    connect(op, &MessageOperation::messageUpdate, this, &Self::updateMessage);
    connect(m_messenger, &Messenger::onlineStatusChanged, op, &NetworkOperation::setIsOnline);
    connect(op, &MessageOperation::notificationCreated, this, &MessagesQueue::notificationCreated);
    connect(this, &MessagesQueue::stopRequested, op, &MessageOperation::stop);
    if (auto download = messageSource->download()) {
        if (download->type == DownloadParameter::Type::Download) {
            m_factory->populateDownload(op, download->filePath);
        } else {
            m_factory->populatePreload(op);
        }
    } else {
        m_factory->populateAll(op);
    }
    return op;
}

void Self::invalidateOperation(OperationSourcePtr source)
{
    const auto messageSource = dynamic_cast<MessageOperationSource *>(source.get());
    const auto message = messageSource->message();

    if (message->isIncoming()) {
        IncomingMessageStageUpdate update;
        update.messageId = message->id();
        update.stage = IncomingMessageStage::Broken;
        emit updateMessage(update);
    } else if (message->isOutgoing()) {
        OutgoingMessageStageUpdate update;
        update.messageId = message->id();
        update.stage = OutgoingMessageStage::Broken;
        emit updateMessage(update);
    }
}

qsizetype Self::maxAttemptCount() const
{
    return 3;
}

void Self::onDatabaseOpened()
{
    start();
    connect(m_userDatabase->messagesTable(), &MessagesTable::notSentMessagesFetched, this,
            &Self::onNotSentMessagesFetched);
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

void Self::onPushMessage(const ModifiableMessageHandler &message)
{
    if (message->isIncoming() || message->isOutgoingCopyFromOtherDevice()) {
        if (message->contentType() == MessageContentType::Picture) {
            pushMessagePreload(message);
        }
    } else {
        addSource(std::make_shared<MessageOperationSource>(message));
    }
}

void Self::onPushMessageDownload(const ModifiableMessageHandler &message, const QString &filePath,
                                 const PostFunction &postFunction)
{
    Q_ASSERT(message->contentIsAttachment());
    DownloadParameter parameter;
    parameter.type = DownloadParameter::Type::Download;
    parameter.filePath = filePath;
    parameter.postFunction = postFunction;
    addSource(std::make_shared<MessageOperationSource>(message, std::move(parameter)));
}

void Self::onPushMessagePreload(const ModifiableMessageHandler &message)
{
    Q_ASSERT(message->contentType() == MessageContentType::Picture);
    DownloadParameter parameter;
    parameter.type = DownloadParameter::Type::Preload;
    addSource(std::make_shared<MessageOperationSource>(message, std::move(parameter)));
}
