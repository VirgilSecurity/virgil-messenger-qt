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

using namespace vm;

UsersController::UsersController(VSQMessenger *messenger, QObject *parent)
    : QObject(parent)
    , m_messenger(messenger)
{
    connect(m_messenger, &VSQMessenger::signedIn, this, &UsersController::signedIn);
    connect(m_messenger, &VSQMessenger::signInErrorOccured, this, &UsersController::signInErrorOccured);
    connect(m_messenger, &VSQMessenger::signedUp, this, &UsersController::signedUp);
    connect(m_messenger, &VSQMessenger::signUpErrorOccured, this, &UsersController::signUpErrorOccured);
    connect(m_messenger, &VSQMessenger::signedOut, this, &UsersController::signedOut);

    connect(m_messenger, &VSQMessenger::keyDownloaded, this, &UsersController::keyDownloaded);
    connect(m_messenger, &VSQMessenger::downloadKeyFailed, this, &UsersController::downloadKeyFailed);

    connect(this, &UsersController::signedIn, this, &UsersController::setUsername);
    connect(this, &UsersController::signedUp, this, &UsersController::setUsername);
    connect(this, &UsersController::keyDownloaded, this, &UsersController::setUsername);
}

void UsersController::signIn(const QString &username)
{
    m_messenger->signIn(username);
}

void UsersController::signOut()
{
    m_messenger->signOut();
}

void UsersController::signUp(const QString &username)
{
    m_messenger->signUp(username);
}

void UsersController::downloadKey(const QString &username, const QString &password)
{
    m_messenger->downloadKey(username, password);
}

void UsersController::setUsername(const QString &username)
{
    if (m_username == username) {
        return;
    }
    m_username = username;
    emit usernameChanged(username);
}
