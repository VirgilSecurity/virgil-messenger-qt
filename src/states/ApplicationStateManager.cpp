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

Q_LOGGING_CATEGORY(lcAppState, "appState");

using namespace VSQ;

ApplicationStateManager::ApplicationStateManager(VSQMessenger *messenger, VSQSettings *settings, QObject *parent)
    : QStateMachine(parent)
    , m_messenger(messenger)
    , m_settings(settings)
    , m_accountSelectionState(new AccountSelectionState(messenger, this))
    , m_accountSettingsState(new AccountSettingsState(this))
    , m_attachmentPreviewState(new AttachmentPreviewState(this))
    , m_backupKeyState(new BackupKeyState(m_messenger, this))
    , m_chatListState(new ChatListState(this))
    , m_chatState(new ChatState(this))
    , m_newChatState(new NewChatState(messenger, this))
    , m_signInAsState(new SignInAsState(messenger, this))
    , m_signInState(new SignInState(messenger, this))
    , m_signUpState(new SignUpState(messenger, this))
    , m_splashScreenState(new SplashScreenState(messenger, settings, this))
    , m_startState(new StartState(this))
{
    registerStatesMetaTypes();
    setupConnections();
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
    qRegisterMetaType<NewChatState *>("NewChatState*");
    qRegisterMetaType<SignInAsState *>("SignInAsState*");
    qRegisterMetaType<SignInState *>("SignInState*");
    qRegisterMetaType<SignUpState *>("SignUpState*");
    qRegisterMetaType<SplashScreenState *>("SplashScreenState*");
    qRegisterMetaType<StartState *>("StartState*");
}

void ApplicationStateManager::setupConnections()
{
    for (auto state : findChildren<QState *>()) {
        connect(state, &QState::entered, this, std::bind(&ApplicationStateManager::setCurrentState, this, state));
        connect(state, &QState::exited, this, std::bind(&ApplicationStateManager::setPreviousState, this, state));
    }

    // NOTE: Queued connection is needed for correct state transition
    connect(this, &ApplicationStateManager::setUiState, this, std::bind(&ApplicationStateManager::splashScreenRequested, this, QPrivateSignal()), Qt::QueuedConnection);
    connect(this, &ApplicationStateManager::signIn, this, &ApplicationStateManager::onSignIn);
    connect(this, &ApplicationStateManager::signUp, this, &ApplicationStateManager::onSignUp);
    connect(this, &ApplicationStateManager::signOut, m_messenger, &VSQMessenger::signOut);
    connect(this, &ApplicationStateManager::sendMessage, m_messenger, &VSQMessenger::sendMessage);
    connect(this, &ApplicationStateManager::openSignInAs, m_signInState, &SignInState::signIn);
    connect(this, &ApplicationStateManager::openChat, this, &ApplicationStateManager::onOpenChat);
    connect(this, &ApplicationStateManager::addContact, m_newChatState, &NewChatState::addContact);
    connect(this, &ApplicationStateManager::backupKey, m_backupKeyState, &BackupKeyState::backupKey);
    connect(m_newChatState, &NewChatState::addContactFinished, this, &ApplicationStateManager::openChat);
    connect(m_messenger, &VSQMessenger::openPreviewRequested, this, &ApplicationStateManager::onOpenPreview);
}

void ApplicationStateManager::addTransitions()
{
    m_startState->addTransition(this, &ApplicationStateManager::splashScreenRequested, m_splashScreenState);
    m_splashScreenState->addTransition(m_splashScreenState, &SplashScreenState::signInUserNotSelected, m_accountSelectionState);
    m_splashScreenState->addTransition(m_splashScreenState, &SplashScreenState::signInErrorOccurred, m_accountSelectionState);
    m_splashScreenState->addTransition(m_splashScreenState, &SplashScreenState::signInFinished, m_chatListState);
    m_accountSelectionState->addTransition(m_accountSelectionState, &AccountSelectionState::signInFinished, m_chatListState);
    addTwoSideTransition(m_accountSelectionState, this, &ApplicationStateManager::openSignIn, m_signInState);
    addTwoSideTransition(m_accountSelectionState, this, &ApplicationStateManager::openSignUp, m_signUpState);
    m_chatListState->addTransition(m_messenger, &VSQMessenger::signedOut, m_accountSelectionState);
    addTwoSideTransition(m_chatListState, this, &ApplicationStateManager::openAccountSettings, m_accountSettingsState);
    addTwoSideTransition(m_chatListState, this, &ApplicationStateManager::openAddContact, m_newChatState);
    addTwoSideTransition(m_chatListState, this, &ApplicationStateManager::chatRequested, m_chatState);
    addTwoSideTransition(m_accountSettingsState, this, &ApplicationStateManager::openBackupKey, m_backupKeyState);
    m_accountSettingsState->addTransition(m_messenger, &VSQMessenger::signedOut, m_accountSelectionState);
    m_newChatState->addTransition(this, &ApplicationStateManager::chatRequested, m_chatState);
    addTwoSideTransition(m_chatState, this, &ApplicationStateManager::openPreviewRequested, m_attachmentPreviewState);
    m_signUpState->addTransition(m_signUpState, &SignUpState::signUpFinished, m_chatListState);
    addTwoSideTransition(m_signInState, m_signInState, &SignInState::signInFinished, m_signInAsState);
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

void ApplicationStateManager::onSignIn(const QString &userId)
{
    const auto authorizationState = dynamic_cast<AuthorizationState *>(m_currentState);
    if (authorizationState) {
        authorizationState->signIn(userId);
    }
}

void ApplicationStateManager::onSignUp(const QString &userId)
{
    const auto authorizationState = dynamic_cast<AuthorizationState *>(m_currentState);
    if (authorizationState) {
        authorizationState->signUp(userId);
    }
}

void ApplicationStateManager::onOpenChat(const QString &contactId)
{
    m_chatState->setContactId(contactId);
    emit chatRequested(QPrivateSignal());
}

void ApplicationStateManager::onOpenPreview(const QUrl &url)
{
    m_attachmentPreviewState->setUrl(url);
    emit openPreviewRequested(QPrivateSignal());
}
