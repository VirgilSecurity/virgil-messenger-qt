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
#include "operations/DownloadAttachmentOperation.h"
#include "operations/MessageOperation.h"
#include "operations/MessageOperationFactory.h"
#include "operations/Operation.h"

using namespace vm;

MessagesQueue::MessagesQueue(const VSQSettings *settings, VSQMessenger *messenger, UserDatabase *userDatabase, FileLoader *fileLoader, QObject *parent)
    : Operation("Queue", parent)
    , m_messenger(messenger)
    , m_userDatabase(userDatabase)
    , m_factory(new MessageOperationFactory(settings, messenger, fileLoader, this))
{
    setRepeatable(true);
    //
    connect(userDatabase, &UserDatabase::userIdChanged, this, &MessagesQueue::setUserId);
    //
    connect(this, &MessagesQueue::pushMessageOperation, this, &MessagesQueue::onPushMessage);
    connect(this, &MessagesQueue::pushDownloadOperation, this, &MessagesQueue::onPushDownloadOperation);
    connect(this, &MessagesQueue::sendNotSentMessages, this, &MessagesQueue::onSendNotSentMessages);
    connect(fileLoader, &FileLoader::ready, this, &MessagesQueue::onSendNotSentMessages);
}

MessagesQueue::~MessagesQueue()
{
}

void MessagesQueue::setUserId(const UserId &userId)
{
    if (m_userId == userId) {
        return;
    }

    dropChildren();
    m_userId = userId;
    if (!userId.isEmpty()) {
        connect(m_userDatabase->messagesTable(), &MessagesTable::notSentMessagesFetched, this, &MessagesQueue::setMessages);
        sendNotSentMessages();
    }
}

void MessagesQueue::setMessages(const GlobalMessages &messages)
{
    qCDebug(lcOperation) << "Queued" << messages.size() << "unsent messages";
    for (auto &m : messages) {
        appendMessageOperation(m);
    }
    if (hasChildren()) {
        start();
    }
}

void MessagesQueue::connectMessageOperation(MessageOperation *op)
{
    connect(op, &MessageOperation::statusChanged, this, std::bind(&MessagesQueue::onMessageOperationStatusChanged, this, op));
    connect(op, &MessageOperation::attachmentStatusChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentStatusChanged, this, op));
    connect(op, &MessageOperation::attachmentProgressChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentProgressChanged, this, op));
    connect(op, &MessageOperation::attachmentUrlChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentUrlChanged, this, op));
    connect(op, &MessageOperation::attachmentExtrasChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentExtrasChanged, this, op));
    connect(op, &MessageOperation::attachmentLocalPathChanged, this, std::bind(&MessagesQueue::onMessageOperationAttachmentLocalPathChanged, this, op));
}

void MessagesQueue::appendMessageOperation(const GlobalMessage &message)
{
    auto op = new MessageOperation(message, m_factory, this);
    connectMessageOperation(op);
    appendChild(op);
}

void MessagesQueue::onPushMessage(const GlobalMessage &message)
{
    appendMessageOperation(message);
    start();
}

void MessagesQueue::onPushDownloadOperation(const GlobalMessage &message, const QString &filePath)
{
    auto op = new DownloadAttachmentOperation(message, m_factory, this, filePath);
    connectMessageOperation(op);
    connect(op, &Operation::finished, this, std::bind(&MessagesQueue::notificationCreated, this, tr("File was downloaded")));
    prependChild(op);
    start();
}

void MessagesQueue::onSendNotSentMessages()
{
    if (!m_userId.isEmpty()) {
        m_userDatabase->messagesTable()->fetchNotSentMessages();
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

void MessagesQueue::onMessageOperationAttachmentProgressChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentProgressChanged(a.id, m.contactId, a.bytesLoaded, a.bytesTotal);
}

void MessagesQueue::onMessageOperationAttachmentUrlChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentUrlChanged(a.id, m.contactId, a.url);
}

void MessagesQueue::onMessageOperationAttachmentExtrasChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentExtrasChanged(a.id, m.contactId, a.type, a.extras);
}

void MessagesQueue::onMessageOperationAttachmentLocalPathChanged(const MessageOperation *operation)
{
    const auto &m = *operation->message();
    const auto &a = *m.attachment;
    emit attachmentLocalPathChanged(a.id, m.contactId, a.localPath);
}
