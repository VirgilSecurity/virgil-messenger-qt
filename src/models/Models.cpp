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

#include "models/Models.h"

#include <QSortFilterProxyModel>
#include <QThread>

#include "Settings.h"
#include "VSQMessenger.h"
#include "models/AttachmentsModel.h"
#include "models/ChatsModel.h"
#include "models/MessagesModel.h"
#include "models/MessagesQueue.h"
#include "models/FileLoader.h"

using namespace vm;

Models::Models(VSQMessenger *messenger, Settings *settings, UserDatabase *userDatabase, QNetworkAccessManager *networkAccessManager, QObject *parent)
    : QObject(parent)
    , m_attachments(new AttachmentsModel(settings, this))
    , m_chats(new ChatsModel(this))
    , m_messages(new MessagesModel(this))
    , m_fileLoader(new FileLoader(messenger->xmpp(), networkAccessManager, this))
    , m_messagesQueue(new MessagesQueue(settings, messenger, userDatabase, m_fileLoader, nullptr))
    , m_queueThread(new QThread())
{
    qRegisterMetaType<AttachmentsModel *>("AttachmentsModel*");
    qRegisterMetaType<ChatsModel *>("ChatsModel*");
    qRegisterMetaType<MessagesModel *>("MessagesModel*");
    qRegisterMetaType<QSortFilterProxyModel *>("QSortFilterProxyModel*");

    connect(m_messagesQueue, &MessagesQueue::notificationCreated, this, &Models::notificationCreated);

    m_messagesQueue->moveToThread(m_queueThread);
    m_queueThread->setObjectName("QueueThread");
    m_queueThread->start();
}

Models::~Models()
{
    m_queueThread->quit();
    m_queueThread->wait();
    delete m_messagesQueue;
    delete m_queueThread;
}

const AttachmentsModel *Models::attachments() const
{
    return m_attachments;
}

AttachmentsModel *Models::attachments()
{
    return m_attachments;
}

const ChatsModel *Models::chats() const
{
    return m_chats;
}

ChatsModel *Models::chats()
{
    return m_chats;
}

const MessagesModel *Models::messages() const
{
    return m_messages;
}

MessagesModel *Models::messages()
{
    return m_messages;
}

const MessagesQueue *Models::messagesQueue() const
{
    return m_messagesQueue;
}

MessagesQueue *Models::messagesQueue()
{
    return m_messagesQueue;
}

const FileLoader *Models::fileLoader() const
{
    return m_fileLoader;
}

FileLoader *Models::fileLoader()
{
    return m_fileLoader;
}
