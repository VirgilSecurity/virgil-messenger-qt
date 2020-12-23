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
#include "FileLoader.h"
#include "MessageOperation.h"
#include "MessageOperationFactory.h"
#include "Operation.h"


using namespace vm;
using Self = MessagesQueue;


Q_LOGGING_CATEGORY(lcMessagesQueue, "messages-queue");


Self::MessagesQueue(const Settings *settings, Messenger *messenger, UserDatabase *userDatabase, QObject *parent)
    : NetworkOperation(parent, messenger->isOnline())
    , m_messenger(messenger)
    , m_userDatabase(userDatabase)
    , m_factory(new MessageOperationFactory(settings, messenger, this))
{
    setName(QLatin1String("MessageQueue"));

    //
    //  Connect database.
    //
    connect(m_userDatabase, &UserDatabase::opened, this, &Self::onDatabaseOpened);

    connect(messenger, &Messenger::onlineStatusChanged, this, &NetworkOperation::setIsOnline);
    connect(this, &Self::pushMessage, this, &Self::onPushMessage);
    connect(this, &Self::pushMessageDownload, this, &Self::onPushMessageDownload);
    connect(this, &Self::pushMessagePreload, this, &Self::onPushMessagePreload);
    connect(messenger->fileLoader(), &FileLoader::uploadServiceFound, this, &Self::onFileLoaderServiceFound);
    connect(this, &Operation::finished, this, &Self::onFinished);
}

MessagesQueue::~MessagesQueue() {
}


void Self::onDatabaseOpened() {
    connect(m_userDatabase->messagesTable(), &MessagesTable::notSentMessagesFetched, this, &Self::onNotSentMessagesFetched);
    m_userDatabase->messagesTable()->fetchNotSentMessages();
}


MessageOperation *Self::pushMessageOperation(const MessageHandler &message)
{
    auto op = new MessageOperation(message, m_factory, this);

    connect(op, &MessageOperation::messageUpdate, this, &Self::updateMessage);
    connect(op, &MessageOperation::notificationCreated, this, &Self::notificationCreated);

    appendChild(op);

    return op;
}


void MessagesQueue::onFileLoaderServiceFound(bool serviceFound)
{
    // FIXME(fpohtmeh): remove method?
    Q_UNUSED(serviceFound)
}


void Self::onNotSentMessagesFetched(const ModifiableMessages &messages)
{
    qCDebug(lcMessagesQueue) << "Queued" << messages.size() << "unsent messages";
    for (auto &m : messages) {
        if (auto op = pushMessageOperation(m)) {
            m_factory->populateAll(op);
        }
    }
}

void Self::onFinished()
{
    qCDebug(lcMessagesQueue) << "Messages queue is finished";
}

void Self::onPushMessage(const MessageHandler &message)
{
    if (auto op = pushMessageOperation(message)) {
        m_factory->populateAll(op);
    }
}

void Self::onPushMessageDownload(const MessageHandler &message, const QString &filePath)
{
    if (auto op = pushMessageOperation(message)) {
        m_factory->populateDownload(op, filePath);
        connect(op, &Operation::finished, this, std::bind(&Self::notificationCreated, this, tr("File was downloaded"), false));
    }
}

void Self::onPushMessagePreload(const MessageHandler &message)
{
    if (auto op = pushMessageOperation(message)) {
        m_factory->populatePreload(op);
    }
}
