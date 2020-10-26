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

#include "controllers/UsersController.h"

#include "VSQMessenger.h"
#include "database/UserDatabase.h"

using namespace vm;

UsersController::UsersController(VSQMessenger *messenger, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
    , m_userDatabase(userDatabase)
{
    connect(messenger, &VSQMessenger::signedIn, this, std::bind(&UsersController::processMessengerOperation, this, args::_1, Operation::SignIn));
    connect(messenger, &VSQMessenger::signInErrorOccured, this, &UsersController::signInErrorOccured);
    connect(messenger, &VSQMessenger::signedUp, this, std::bind(&UsersController::processMessengerOperation, this, args::_1, Operation::SignUp));
    connect(messenger, &VSQMessenger::signUpErrorOccured, this, &UsersController::signUpErrorOccured);
    connect(messenger, &VSQMessenger::signedOut, this, std::bind(&UsersController::processMessengerOperation, this, QString(), Operation::SignOut));

    connect(messenger, &VSQMessenger::keyDownloaded, this, std::bind(&UsersController::processMessengerOperation, this, args::_1, Operation::DownloadKey));
    connect(messenger, &VSQMessenger::downloadKeyFailed, this, &UsersController::downloadKeyFailed);
}

QString UsersController::username() const
{
    return m_username;
}

void UsersController::signIn(const QString &username)
{
    m_messenger->signIn(username);
}

void UsersController::signUp(const QString &username)
{
    m_messenger->signUp(username);
}

void UsersController::signOut()
{
    m_messenger->signOut();
}

void UsersController::downloadKey(const QString &username, const QString &password)
{
    m_messenger->downloadKey(username, password);
}

void UsersController::processMessengerOperation(const QString &username, const Operation operation)
{
    disconnect(m_databaseConnection);
    auto slot = std::bind(&UsersController::processDatabaseOperation, this, username, operation);
    if (operation == Operation::SignOut) {
        m_databaseConnection = connect(m_userDatabase, &UserDatabase::closed, this, slot);
        m_userDatabase->requestClose();
    }
    else {
        m_databaseConnection = connect(m_userDatabase, &UserDatabase::opened, this, slot);
        m_userDatabase->requestOpen(username);
    }
}

void UsersController::processDatabaseOperation(const QString &username, const Operation operation)
{
    if (m_username != username) {
        m_username = username;
        emit usernameChanged(username);
    }

    switch (operation) {
    case Operation::SignIn:
        emit signedIn(username);
        break;
    case Operation::SignUp:
        emit signedUp(username);
        break;
    case Operation::SignOut:
        emit signedOut();
        break;
    case Operation::DownloadKey:
        emit keyDownloaded(username);
    default:
        break;
    }
}
