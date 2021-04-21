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

#ifndef VM_USERSCONTROLLER_H
#define VM_USERSCONTROLLER_H

#include "UserId.h"
#include "Chat.h"
#include "Messenger.h"

#include <QObject>
#include <QPointer>

class Messenger;

namespace vm {
class Models;
class UserDatabase;

class UsersController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentUserId READ currentUserId NOTIFY currentUserIdChanged)
    Q_PROPERTY(QString currentUsername READ currentUsername NOTIFY currentUsernameChanged)

public:
    UsersController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent);

signals:
    void notificationCreated(const QString &notification, const bool error) const;

    void currentUserIdChanged(const QString &userId);
    void currentUsernameChanged(const QString &username);

private:
    QString currentUserId() const;
    QString currentUsername() const;

    void updateCurrentUser();
    void updateContactsWithUser(const UserHandler &user);

    void onChatAdded(const ChatHandler &chat);

private:
    QPointer<Messenger> m_messenger;
    QPointer<UserDatabase> m_userDatabase;
};
} // namespace vm

#endif // VM_USERSCONTROLLER_H
