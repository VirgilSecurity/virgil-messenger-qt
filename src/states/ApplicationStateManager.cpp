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

#include "VSQMessenger.h"
#include "controllers/Controllers.h"
#include "controllers/UsersController.h"

Q_LOGGING_CATEGORY(lcAppState, "appState");

using namespace vm;

ApplicationStateManager::ApplicationStateManager(VSQMessenger *messenger, Controllers *controllers, Validator *validator, VSQSettings *settings, QObject *parent)
    : QStateMachine(parent)
    , m_messenger(messenger)
    , m_controllers(controllers)
    , m_validator(validator)
    , m_settings(settings)
    , m_accountSelectionState(new AccountSelectionState(controllers->users(), validator, settings, this))
    , m_accountSettingsState(new AccountSettingsState(this))
    , m_attachmentPreviewState(new AttachmentPreviewState(this))
    , m_backupKeyState(new BackupKeyState(m_messenger, this))
    , m_chatListState(new ChatListState(controllers, this))
    , m_chatState(new ChatState(m_messenger, this))
    , m_downloadKeyState(new DownloadKeyState(controllers->users(), this))
    , m_newChatState(new NewChatState(controllers->chats(), this))
    , m_signInAsState(new SignInAsState(this))
    , m_signInUsernameState(new SignInUsernameState(validator, this))
    , m_signUpState(new SignUpState(controllers->users(), validator, this))
    , m_splashScreenState(new SplashScreenState(controllers->users(), validator, settings, this))
    , m_startState(new StartState(this))
{
    registerStatesMetaTypes();
    addTransitions();
    setInitialState(m_startState);
    start();
}

ApplicationStateManager::~ApplicationStateManager()
{}

void ApplicationStateManager::registerStatesMetaTypes()
{
    // Qt requires registering to avoid namespace issues
    qRegisterMetaType<AccountSelectionState *>("AccountSelectionState*");
    qRegisterMetaType<AccountSettingsState *>("AccountSettingsState*");
    qRegisterMetaType<AttachmentPreviewState *>("AttachmentPreviewState*");
    qRegisterMetaType<BackupKeyState *>("BackupKeyState*");
    qRegisterMetaType<ChatListState *>("ChatListState*");
    qRegisterMetaType<ChatState *>("ChatState*");
    qRegisterMetaType<DownloadKeyState *>("DownloadKeyState*");
    qRegisterMetaType<NewChatState *>("NewChatState*");
    qRegisterMetaType<SignInAsState *>("SignInAsState*");
    qRegisterMetaType<SignInUsernameState *>("SignInUsernameState*");
    qRegisterMetaType<SignUpState *>("SignUpState*");
    qRegisterMetaType<SplashScreenState *>("SplashScreenState*");
    qRegisterMetaType<StartState *>("StartState*");
}

void ApplicationStateManager::addTransitions()
{
    for (auto state : findChildren<QState *>()) {
        connect(state, &QState::entered, this, std::bind(&ApplicationStateManager::setCurrentState, this, state));
        connect(state, &QState::exited, this, std::bind(&ApplicationStateManager::setPreviousState, this, state));
    }

    m_startState->addTransition(this, &ApplicationStateManager::splashScreenRequested, m_splashScreenState);
    // NOTE: Queued connection is a workaround for working state transition
    connect(this, &ApplicationStateManager::setUiState, this, std::bind(&ApplicationStateManager::splashScreenRequested, this, QPrivateSignal()), Qt::QueuedConnection);

    m_splashScreenState->addTransition(m_splashScreenState, &SplashScreenState::userNotSelected, m_accountSelectionState);
    m_splashScreenState->addTransition(m_splashScreenState, &SplashScreenState::operationErrorOccurred, m_accountSelectionState);
    m_splashScreenState->addTransition(m_controllers->users(), &UsersController::signedIn, m_chatListState);

    m_accountSelectionState->addTransition(m_controllers->users(), &UsersController::signedIn, m_chatListState);
    addTwoSideTransition(m_accountSelectionState, m_accountSelectionState, &AccountSelectionState::requestSignInUsername, m_signInUsernameState);
    addTwoSideTransition(m_accountSelectionState, m_accountSelectionState, &AccountSelectionState::requestSignUp, m_signUpState);

    m_chatListState->addTransition(m_controllers->users(), &UsersController::signedOut, m_accountSelectionState);
    addTwoSideTransition(m_chatListState, m_chatListState, &ChatListState::requestAccountSettings, m_accountSettingsState);
    connect(m_chatListState, &ChatListState::requestAccountSettings, m_accountSettingsState, &AccountSettingsState::setUserId);
    addTwoSideTransition(m_chatListState, m_chatListState, &ChatListState::requestNewChat, m_newChatState);
    addTwoSideTransition(m_chatListState, m_chatListState, &ChatListState::requestChat, m_chatState);
    connect(m_chatListState, &ChatListState::requestChat, m_chatState, &ChatState::setContactId);

    addTwoSideTransition(m_accountSettingsState, m_accountSettingsState, &AccountSettingsState::requestBackupKey, m_backupKeyState);
    connect(m_accountSettingsState, &AccountSettingsState::requestBackupKey, m_backupKeyState, &BackupKeyState::setUserId);
    m_accountSettingsState->addTransition(m_controllers->users(), &UsersController::signedOut, m_accountSelectionState);

    m_newChatState->addTransition(m_newChatState, &NewChatState::requestChat, m_chatState);
    connect(m_newChatState, &NewChatState::requestChat, m_chatState, &ChatState::setContactId);

    addTwoSideTransition(m_chatState, m_chatState, &ChatState::requestPreview, m_attachmentPreviewState);
    connect(m_chatState, &ChatState::requestPreview, m_attachmentPreviewState, &AttachmentPreviewState::setUrl);

    m_signUpState->addTransition(m_controllers->users(), &UsersController::signedUp, m_chatListState);

    addTwoSideTransition(m_signInUsernameState, m_signInUsernameState, &SignInUsernameState::validated, m_signInAsState);
    connect(m_signInUsernameState, &SignInUsernameState::validated, m_signInAsState, &SignInAsState::setUserId);

    addTwoSideTransition(m_signInAsState, m_signInAsState, &SignInAsState::requestDownloadKey, m_downloadKeyState);
    connect(m_signInAsState, &SignInAsState::requestDownloadKey, m_downloadKeyState, &DownloadKeyState::setUserId);

    m_downloadKeyState->addTransition(m_controllers->users(), &UsersController::keyDownloaded, m_chatListState);
}

void ApplicationStateManager::setCurrentState(QState *state)
{
    qCDebug(lcAppState) << "Current state:" << state;
    m_currentState = state;
    emit currentStateChanged(state);
}

void ApplicationStateManager::setPreviousState(QState *state)
{
    m_previousState = state;
    emit previousStateChanged(state);
}
