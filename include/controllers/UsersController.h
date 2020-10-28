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

#include <QObject>

#include "VSQCommon.h"

class VSQMessenger;

namespace vm
{
class UserDatabase;

class UsersController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username MEMBER m_username NOTIFY usernameChanged);

public:
    UsersController(VSQMessenger *messenger, UserDatabase *userDatabase, QObject *parent);

    QString username() const; // FIXME(fpohtmeh): use userId instead of username

    void signIn(const QString &username);
    void signUp(const QString &username);
    Q_INVOKABLE void signOut();

    void downloadKey(const QString &username, const QString &password);

    void subscribeToContact(const Contact::Id &contactId);

signals:
    void signedIn(const QString &username);
    void signInErrorOccured(const QString &errorText);
    void signedUp(const QString &username);
    void signUpErrorOccured(const QString &errorText);
    void signedOut();

    void keyDownloaded(const QString &username);
    void downloadKeyFailed(const QString &errorText);

    void usernameChanged(const QString &username);

private:
    enum class Operation
    {
        SignIn,
        SignUp,
        SignOut,
        DownloadKey
    };

    void openDatabase(const QString &username, const Operation operation);
    void processDatabaseUsername(const QString &username);

    VSQMessenger *m_messenger;
    UserDatabase *m_userDatabase;
    Operation m_operation = Operation::SignIn;
    QString m_username;
};
}

#endif // VM_USERSCONTROLLER_H
