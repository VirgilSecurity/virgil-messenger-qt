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

#include "states/ApplicationStateManager.h"

#include "Messenger.h"
#include "controllers/Controllers.h"
#include "controllers/ChatsController.h"
#include "controllers/UsersController.h"
#include "models/Models.h"

#include <QLoggingCategory>

using namespace vm;
using Self = ApplicationStateManager;


Q_LOGGING_CATEGORY(lcAppState, "app-state");


Self::ApplicationStateManager(Messenger *messenger, Controllers *controllers, Models *models, Validator *validator, Settings *settings, QObject *parent)
    : QStateMachine(parent)
    , m_messenger(messenger)
    , m_controllers(controllers)
    , m_validator(validator)
    , m_settings(settings)
    , m_accountSelectionState(new AccountSelectionState(controllers->users(), validator, this))
    , m_accountSettingsState(new AccountSettingsState(this))
    , m_attachmentPreviewState(new AttachmentPreviewState(this))
    , m_backupKeyState(new BackupKeyState(m_messenger, this))
    , m_editProfileState(new EditProfileState(controllers->users(), this))
    , m_verifyProfileState(new VerifyProfileState(this))
    , m_chatListState(new ChatListState(controllers->chats(), this))
    , m_chatState(new ChatState(controllers, m_messenger, this))
    , m_downloadKeyState(new DownloadKeyState(controllers->users(), this))
    , m_fileCloudState(new FileCloudState(models, this))
    , m_newChatState(new NewChatState(controllers->chats(), models->discoveredContacts(), this))
    , m_newGroupChatState(new NewGroupChatState(models->discoveredContacts(), this))
    , m_nameGroupChatState(new NameGroupChatState(controllers->chats(), this))
    , m_signInAsState(new SignInAsState(this))
    , m_signInUsernameState(new SignInUsernameState(controllers->users(), validator, this))
    , m_signUpState(new SignUpState(controllers->users(), validator, this))
    , m_splashScreenState(new SplashScreenState(controllers->users(), validator, settings, this))
    , m_startState(new StartState(this))
{
    registerStatesMetaTypes();
    addTransitions();
    setInitialState(m_startState);
    start();
}

Self::~ApplicationStateManager()
{}

void Self::registerStatesMetaTypes()
{
    // Qt requires registering to avoid namespace issues
    qRegisterMetaType<AccountSelectionState *>("AccountSelectionState*");
    qRegisterMetaType<AccountSettingsState *>("AccountSettingsState*");
    qRegisterMetaType<AttachmentPreviewState *>("AttachmentPreviewState*");
    qRegisterMetaType<BackupKeyState *>("BackupKeyState*");
    qRegisterMetaType<EditProfileState *>("EditProfileState*");
    qRegisterMetaType<VerifyProfileState *>("VerifyProfileState*");
    qRegisterMetaType<ChatListState *>("ChatListState*");
    qRegisterMetaType<ChatState *>("ChatState*");
    qRegisterMetaType<DownloadKeyState *>("DownloadKeyState*");
    qRegisterMetaType<FileCloudState *>("FileCloudState*");
    qRegisterMetaType<NewChatState *>("NewChatState*");
    qRegisterMetaType<NewGroupChatState *>("NewGroupChatState*");
    qRegisterMetaType<NameGroupChatState *>("NameGroupChatState*");
    qRegisterMetaType<SignInAsState *>("SignInAsState*");
    qRegisterMetaType<SignInUsernameState *>("SignInUsernameState*");
    qRegisterMetaType<SignUpState *>("SignUpState*");
    qRegisterMetaType<SplashScreenState *>("SplashScreenState*");
    qRegisterMetaType<StartState *>("StartState*");
}

void Self::addTransitions()
{
    for (auto state : findChildren<QState *>()) {
        connect(state, &QState::entered, this, std::bind(&Self::setCurrentState, this, state));
        connect(state, &QState::exited, this, std::bind(&Self::setPreviousState, this, state));
    }

    auto users = m_controllers->users();
    auto chats = m_controllers->chats();

    m_startState->addTransition(this, &Self::splashScreenRequested, m_splashScreenState);
    // NOTE: Queued connection is a workaround for working state transition
    connect(this, &Self::setUiState, this, std::bind(&Self::splashScreenRequested, this, QPrivateSignal()), Qt::QueuedConnection);
    connect(this, &Self::openChatList, this, std::bind(&Self::chatListRequested, this, QPrivateSignal()), Qt::QueuedConnection);
    connect(this, &Self::openFileCloud, this, std::bind(&Self::fileCloudRequested, this, QPrivateSignal()), Qt::QueuedConnection);

    m_splashScreenState->addTransition(m_splashScreenState, &SplashScreenState::userNotSelected, m_accountSelectionState);
    m_splashScreenState->addTransition(m_splashScreenState, &SplashScreenState::operationErrorOccurred, m_accountSelectionState);
    m_splashScreenState->addTransition(users, &UsersController::signedIn, m_chatListState);

    m_accountSelectionState->addTransition(users, &UsersController::signedIn, m_chatListState);
    addTwoSideTransition(m_accountSelectionState, m_accountSelectionState, &AccountSelectionState::requestSignInUsername, m_signInUsernameState);
    addTwoSideTransition(m_accountSelectionState, m_accountSelectionState, &AccountSelectionState::requestSignUp, m_signUpState);

    m_chatListState->addTransition(users, &UsersController::signedOut, m_accountSelectionState);
    addTwoSideTransition(m_chatListState, users, &UsersController::accountSettingsRequested, m_accountSettingsState);
    addTwoSideTransition(m_chatListState, m_chatListState, &ChatListState::requestNewChat, m_newChatState);
    addTwoSideTransition(m_chatListState, m_chatListState, &ChatListState::requestNewGroupChat, m_newGroupChatState);
    addTwoSideTransition(m_chatListState, chats, &ChatsController::chatOpened, m_chatState);
    m_chatListState->addTransition(this, &Self::fileCloudRequested, m_fileCloudState);

    addTwoSideTransition(m_newGroupChatState, m_newGroupChatState, &NewGroupChatState::requestChatName, m_nameGroupChatState);

    m_nameGroupChatState->addTransition(chats, &ChatsController::chatOpened, m_chatState);

    addTwoSideTransition(m_fileCloudState, users, &UsersController::accountSettingsRequested, m_accountSettingsState);
    m_fileCloudState->addTransition(users, &UsersController::signedOut, m_accountSelectionState);
    m_fileCloudState->addTransition(this, &Self::chatListRequested, m_chatListState);

    addTwoSideTransition(m_accountSettingsState, m_accountSettingsState, &AccountSettingsState::requestBackupKey, m_backupKeyState);
    m_accountSettingsState->addTransition(users, &UsersController::signedOut, m_accountSelectionState);
    addTwoSideTransition(m_accountSettingsState, m_accountSettingsState, &AccountSettingsState::editProfile, m_editProfileState);

    addTwoSideTransition(m_editProfileState, m_editProfileState, &EditProfileState::verify, m_verifyProfileState);
    connect(m_editProfileState, &EditProfileState::verify, m_verifyProfileState, &VerifyProfileState::setCodeType);
    connect(m_verifyProfileState, &VerifyProfileState::verificationFinished, m_editProfileState, &EditProfileState::processVerificationResponse);

    m_newChatState->addTransition(chats, &ChatsController::chatOpened, m_chatState);
    m_newChatState->addTransition(users, &UsersController::accountSettingsRequested, m_accountSettingsState);
    m_newChatState->addTransition(users, &UsersController::signedOut, m_accountSelectionState);

    addTwoSideTransition(m_chatState, m_chatState, &ChatState::requestPreview, m_attachmentPreviewState);
    connect(m_chatState, &ChatState::requestPreview, m_attachmentPreviewState, &AttachmentPreviewState::setUrl);

    m_signUpState->addTransition(users, &UsersController::signedIn, m_chatListState);

    addTwoSideTransition(m_signInUsernameState, m_signInUsernameState, &SignInUsernameState::validated, m_signInAsState);

    addTwoSideTransition(m_signInAsState, m_signInAsState, &SignInAsState::requestDownloadKey, m_downloadKeyState);

    m_downloadKeyState->addTransition(users, &UsersController::signedIn, m_chatListState);
}

void Self::setCurrentState(QState *state)
{
    qCDebug(lcAppState) << "Current state:" << state;
    m_currentState = state;
    emit currentStateChanged(state);
}

void Self::setPreviousState(QState *state)
{
    m_previousState = state;
    emit previousStateChanged(state);
}
