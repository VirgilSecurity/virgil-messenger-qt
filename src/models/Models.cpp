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

#include "AccountSelectionModel.h"
#include "ChatsModel.h"
#include "CloudFilesModel.h"
#include "CloudFilesTransfersModel.h"
#include "CloudFilesQueue.h"
#include "DiscoveredContactsModel.h"
#include "MessagesModel.h"
#include "MessagesQueue.h"
#include "FileLoader.h"
#include "UserDatabase.h"
#include "Messenger.h"

using namespace vm;

Models::Models(Messenger *messenger, Settings *settings, UserDatabase *userDatabase, Validator *validator,
               QObject *parent)
    : QObject(parent),
      m_accountSelection(new AccountSelectionModel(settings, this)),
      m_chats(new ChatsModel(this)),
      m_discoveredContacts(new DiscoveredContactsModel(validator, this)),
      m_messages(new MessagesModel(this)),
      m_cloudFiles(new CloudFilesModel(settings, this)),
      m_cloudFilesTransfers(new CloudFilesTransfersModel(this)),
      m_cloudFilesQueue(new CloudFilesQueue(messenger, userDatabase, this)),
      m_fileLoader(messenger->fileLoader()),
      m_messagesQueue(new MessagesQueue(messenger, userDatabase, this))

{
    connect(m_messagesQueue, &MessagesQueue::notificationCreated, this, &Models::notificationCreated);
    connect(m_cloudFilesQueue, &CloudFilesQueue::notificationCreated, this, &Models::notificationCreated);
}

Models::~Models() {}

const AccountSelectionModel *Models::accountSelection() const
{
    return m_accountSelection;
}

AccountSelectionModel *Models::accountSelection()
{
    return m_accountSelection;
}

const ChatsModel *Models::chats() const
{
    return m_chats;
}

ChatsModel *Models::chats()
{
    return m_chats;
}

const DiscoveredContactsModel *Models::discoveredContacts() const
{
    return m_discoveredContacts;
}

DiscoveredContactsModel *Models::discoveredContacts()
{
    return m_discoveredContacts;
}

const CloudFilesModel *Models::cloudFiles() const
{
    return m_cloudFiles;
}

CloudFilesModel *Models::cloudFiles()
{
    return m_cloudFiles;
}

const CloudFilesTransfersModel *Models::cloudFilesTransfers() const
{
    return m_cloudFilesTransfers;
}

CloudFilesTransfersModel *Models::cloudFilesTransfers()
{
    return m_cloudFilesTransfers;
}

const CloudFilesQueue *Models::cloudFilesQueue() const
{
    return m_cloudFilesQueue;
}

CloudFilesQueue *Models::cloudFilesQueue()
{
    return m_cloudFilesQueue;
}

const FileLoader *Models::fileLoader() const
{
    return m_fileLoader;
}

FileLoader *Models::fileLoader()
{
    return m_fileLoader;
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
