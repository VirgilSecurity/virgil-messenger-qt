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

#ifndef VM_CONTROLLERS_H
#define VM_CONTROLLERS_H

#include <QObject>

class VSQMessenger;
class Settings;

namespace vm
{
class AttachmentsController;
class ChatsController;
class MessagesController;
class Models;
class UserDatabase;
class UsersController;

class Controllers : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AttachmentsController *attachments READ attachments CONSTANT)
    Q_PROPERTY(UsersController *users READ users CONSTANT)
    Q_PROPERTY(ChatsController *chats READ chats CONSTANT)
    Q_PROPERTY(MessagesController *messages READ messages CONSTANT)

public:
    Controllers(VSQMessenger *messenger, Settings *settings,
                Models *models, UserDatabase *userDatabase, QObject *parent);

    const AttachmentsController *attachments() const;
    AttachmentsController *attachments();
    const UsersController *users() const;
    UsersController *users();
    const ChatsController *chats() const;
    ChatsController *chats();
    const MessagesController *messages() const;
    MessagesController *messages();

signals:
    void notificationCreated(const QString &notification);

private:
    AttachmentsController *m_attachments;
    UsersController *m_users;
    ChatsController *m_chats;
    MessagesController *m_messages;
};
}

#endif // VM_CONTROLLERS_H
