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

#ifndef VM_MODELS_H
#define VM_MODELS_H

#include <QObject>

class QNetworkAccessManager;

class VSQMessenger;
class Settings;

namespace vm
{
class AccountSelectionModel;
class AttachmentsModel;
class ChatsModel;
class DiscoveredContactsModel;
class FileCloudModel;
class FileCloudUploader;
class FileLoader;
class MessagesModel;
class MessagesQueue;
class UserDatabase;
class Validator;

class Models : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AccountSelectionModel *accountSelection MEMBER m_accountSelection CONSTANT)
    Q_PROPERTY(AttachmentsModel *attachments READ attachments CONSTANT)
    Q_PROPERTY(ChatsModel *chats READ chats CONSTANT)
    Q_PROPERTY(DiscoveredContactsModel *discoveredContacts MEMBER m_discoveredContacts CONSTANT)
    Q_PROPERTY(FileCloudModel *fileCloud MEMBER m_fileCloud CONSTANT)
    Q_PROPERTY(FileCloudUploader *fileCloudUploader MEMBER m_fileCloudUploader CONSTANT)
    Q_PROPERTY(MessagesModel *messages READ messages CONSTANT)

public:
    Models(VSQMessenger *messenger, Settings *settings, Validator *validator, UserDatabase *userDatabase, QNetworkAccessManager *networkAccessManager, QObject *parent);
    ~Models() override;

    const AccountSelectionModel *accountSelection() const;
    AccountSelectionModel *accountSelection();
    const AttachmentsModel *attachments() const;
    AttachmentsModel *attachments();
    const ChatsModel *chats() const;
    ChatsModel *chats();
    const DiscoveredContactsModel *discoveredContacts() const;
    DiscoveredContactsModel *discoveredContacts();
    const FileCloudModel *fileCloud() const;
    FileCloudModel *fileCloud();
    const FileCloudUploader *fileCloudUploader() const;
    FileCloudUploader *fileCloudUploader();
    const FileLoader *fileLoader() const;
    FileLoader *fileLoader();
    const MessagesModel *messages() const;
    MessagesModel *messages();
    const MessagesQueue *messagesQueue() const;
    MessagesQueue *messagesQueue();

signals:
    void notificationCreated(const QString &notification, const bool error);

private:
    AccountSelectionModel *m_accountSelection;
    AttachmentsModel *m_attachments;
    ChatsModel *m_chats;
    DiscoveredContactsModel *m_discoveredContacts;
    MessagesModel *m_messages;
    FileCloudModel *m_fileCloud;
    FileCloudUploader *m_fileCloudUploader;
    FileLoader *m_fileLoader;
    MessagesQueue *m_messagesQueue;
};
}

#endif // VM_MODELS_H
