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

#include "Messenger.h"
#include "database/UserDatabase.h"
#include "database/ContactsTable.h"
#include "models/Models.h"
#include "models/ChatsModel.h"
#include "models/MessagesModel.h"

#include <utility>

using namespace vm;
using Self = UsersController;

Self::UsersController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent)
    : QObject(parent), m_messenger(messenger), m_userDatabase(userDatabase)
{
    connect(messenger, &Messenger::signedIn, this, &Self::onSignedIn);
    connect(messenger, &Messenger::signedUp, this, &Self::onSignedIn);
    connect(messenger, &Messenger::keyDownloaded, this, &Self::onSignedIn);
    connect(messenger, &Messenger::signedOut, this, &Self::onSignedOut);

    connect(messenger, &Messenger::signInErrorOccured, this, &Self::signInErrorOccured);
    connect(messenger, &Messenger::signUpErrorOccured, this, &Self::signUpErrorOccured);
    connect(messenger, &Messenger::downloadKeyFailed, this, &Self::downloadKeyFailed);

    connect(messenger, &Messenger::userWasFound, this, &Self::onUpdateContactsWithUser);

    connect(userDatabase, &UserDatabase::opened, this, &Self::onFinishSignIn);
    connect(userDatabase, &UserDatabase::closed, this, &Self::onFinishSignOut);
    connect(userDatabase, &UserDatabase::errorOccurred, this, &Self::databaseErrorOccurred);

    auto notifyAboutError = [this](const QString &text) { emit notificationCreated(text, true); };
    connect(this, &Self::signInErrorOccured, notifyAboutError);
    connect(this, &Self::signUpErrorOccured, notifyAboutError);
    connect(this, &Self::downloadKeyFailed, notifyAboutError);
    connect(this, &Self::databaseErrorOccurred, notifyAboutError);

    connect(models->chats(), &ChatsModel::chatAdded, this, &Self::onChatAdded);
}

void Self::signIn(const QString &username)
{
    setNextUsername(username);
    m_messenger->signIn(username);
}

void Self::signUp(const QString &username)
{
    setNextUsername(username);
    m_messenger->signUp(username);
}

void Self::signOut()
{
    m_messenger->signOut();
}

void Self::requestAccountSettings(const QString &username)
{
    emit accountSettingsRequested(username);
}

void Self::downloadKey(const QString &username, const QString &password)
{
    m_messenger->downloadKey(username, password);
}

QString UsersController::currentUserId() const
{
    const auto currentUser = m_messenger->currentUser();
    return currentUser ? currentUser->id() : QString();
}

QString UsersController::currentUsername() const
{
    const auto currentUser = m_messenger->currentUser();
    return currentUser ? currentUser->username() : QString();
}

void UsersController::setNextUsername(const QString &username)
{
    m_nextUsername = username;
    emit nextUsernameChanged(username);
}

QString UsersController::nextUsername() const
{
    return m_nextUsername;
}

void Self::onSignedIn(const QString &username)
{
    m_userDatabase->open(username);
}

void Self::onSignedOut()
{
    m_userDatabase->close();
}

void Self::onFinishSignIn()
{
    const auto user = m_messenger->currentUser();

    onUpdateContactsWithUser(user);

    // TODO: Do we really need to duplicate signals?
    emit signedIn(user->username());

    emit currentUserIdChanged(user->id());
    emit currentUsernameChanged(user->username());
}

void Self::onFinishSignOut()
{
    emit signedOut();
}

void Self::onChatAdded(const ChatHandler &chat)
{
    if (chat->type() == Chat::Type::Personal) {
        m_messenger->subscribeToUser(UserId(chat->id()));
    }
}

void Self::onUpdateContactsWithUser(const UserHandler &user)
{
    Contact contact;
    contact.setUserId(user->id());
    contact.setUsername(user->username());

    m_userDatabase->contactsTable()->addContact(contact);
}
