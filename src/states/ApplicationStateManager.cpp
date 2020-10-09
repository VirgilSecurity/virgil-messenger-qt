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
    , m_accountSelectionState(new AccountSelectionState(messenger, settings, this))
    , m_accountSettingsState(new AccountSettingsState(this))
    , m_attachmentPreviewState(new AttachmentPreviewState(this))
    , m_backupKeyState(new BackupKeyState(m_messenger, this))
    , m_chatListState(new ChatListState(this))
    , m_chatState(new ChatState(m_messenger, this))
    , m_downloadKeyState(new DownloadKeyState(m_messenger, this))
    , m_newChatState(new NewChatState(messenger, this))
    , m_signInAsState(new SignInAsState(this))
    , m_signInUsernameState(new SignInUsernameState(this))
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
    qRegisterMetaType<DownloadKeyState *>("DownloadKeyState*");
    qRegisterMetaType<NewChatState *>("NewChatState*");
    qRegisterMetaType<SignInAsState *>("SignInAsState*");
    qRegisterMetaType<SignInUsernameState *>("SignInUsernameState*");
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
    connect(this, &ApplicationStateManager::addContact, this, &ApplicationStateManager::onAddContact);
    connect(this, &ApplicationStateManager::backupKey, this, &ApplicationStateManager::onBackupKey);
    connect(this, &ApplicationStateManager::downloadKey, this, &ApplicationStateManager::onDownloadKey);
    connect(this, &ApplicationStateManager::openSignInAs, this, &ApplicationStateManager::onOpenSignInAs);
    connect(this, &ApplicationStateManager::openChat, this, &ApplicationStateManager::onOpenChat);
    connect(this, &ApplicationStateManager::openChatList, this, &ApplicationStateManager::onOpenChatList);
    connect(this, &ApplicationStateManager::openDownloadKey, this, &ApplicationStateManager::onOpenDownloadKey);
    connect(this, &ApplicationStateManager::openAccountSettings, this, &ApplicationStateManager::onOpenAccountSettings);
    connect(this, &ApplicationStateManager::openBackupKey, this, &ApplicationStateManager::onOpenBackupKey);
    connect(m_messenger, &VSQMessenger::signedIn, this, &ApplicationStateManager::openChatList);
    connect(m_messenger, &VSQMessenger::signedUp, this, &ApplicationStateManager::openChatList);
    connect(m_messenger, &VSQMessenger::keyBackuped, this, &ApplicationStateManager::openChatList);
    connect(m_messenger, &VSQMessenger::keyDownloaded, this, &ApplicationStateManager::openChatList);
    connect(m_messenger, &VSQMessenger::contactAdded, this, &ApplicationStateManager::openChat);
    connect(m_messenger, &VSQMessenger::openPreviewRequested, this, &ApplicationStateManager::onOpenPreview);
    connect(m_signInUsernameState, &SignInUsernameState::usernameValidated, this, &ApplicationStateManager::onSignInUsernameValidated);
}

void ApplicationStateManager::addTransitions()
{
    m_startState->addTransition(this, &ApplicationStateManager::splashScreenRequested, m_splashScreenState);
    m_splashScreenState->addTransition(m_splashScreenState, &SplashScreenState::userNotSelected, m_accountSelectionState);
    m_splashScreenState->addTransition(m_splashScreenState, &SplashScreenState::operationErrorOccurred, m_accountSelectionState);
    m_splashScreenState->addTransition(this, &ApplicationStateManager::chatListRequested, m_chatListState);
    m_accountSelectionState->addTransition(this, &ApplicationStateManager::chatListRequested, m_chatListState);
    addTwoSideTransition(m_accountSelectionState, this, &ApplicationStateManager::openSignIn, m_signInUsernameState);
    addTwoSideTransition(m_accountSelectionState, this, &ApplicationStateManager::openSignUp, m_signUpState);
    m_chatListState->addTransition(m_messenger, &VSQMessenger::signedOut, m_accountSelectionState);
    addTwoSideTransition(m_chatListState, this, &ApplicationStateManager::openAccountSettingsRequested, m_accountSettingsState);
    addTwoSideTransition(m_chatListState, this, &ApplicationStateManager::openAddContact, m_newChatState);
    addTwoSideTransition(m_chatListState, this, &ApplicationStateManager::chatRequested, m_chatState);
    addTwoSideTransition(m_accountSettingsState, this, &ApplicationStateManager::openBackupKeyRequested, m_backupKeyState);
    m_accountSettingsState->addTransition(m_messenger, &VSQMessenger::signedOut, m_accountSelectionState);
    m_newChatState->addTransition(this, &ApplicationStateManager::chatRequested, m_chatState);
    addTwoSideTransition(m_chatState, this, &ApplicationStateManager::openPreviewRequested, m_attachmentPreviewState);
    m_signUpState->addTransition(this, &ApplicationStateManager::chatListRequested, m_chatListState);
    addTwoSideTransition(m_signInUsernameState, this, &ApplicationStateManager::signInAsRequested, m_signInAsState);
    addTwoSideTransition(m_signInAsState, this, &ApplicationStateManager::keyDownloadRequested, m_downloadKeyState);
    m_downloadKeyState->addTransition(this, &ApplicationStateManager::chatListRequested, m_chatListState);
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
    const auto state = dynamic_cast<SignInState *>(m_currentState);
    if (state) {
        state->signIn(userId);
    }
}

void ApplicationStateManager::onSignUp(const QString &userId)
{
    if (m_currentState == m_signUpState) {
        m_signUpState->signUp(userId);
    }
}

void ApplicationStateManager::onAddContact(const QString &contactId)
{
    if (m_currentState == m_newChatState) {
        m_newChatState->addContact(contactId);
    }
}

void ApplicationStateManager::onBackupKey(const QString &password, const QString &confirmedPassword)
{
    if (m_currentState == m_backupKeyState) {
        m_backupKeyState->backupKey(password, confirmedPassword);
    }
}

void ApplicationStateManager::onDownloadKey(const QString &password)
{
    if (m_currentState == m_downloadKeyState) {
        m_downloadKeyState->downloadKey(password);
    }
}

void ApplicationStateManager::onOpenSignInAs(const QString &userId)
{
    if (m_currentState == m_signInUsernameState) {
        m_signInUsernameState->validateUsername(userId);
    }
}

void ApplicationStateManager::onOpenChat(const QString &contactId)
{
    if (m_currentState == m_chatListState || m_currentState == m_newChatState) {
        m_chatState->setContactId(contactId);
        emit chatRequested(QPrivateSignal());
    }
}

void ApplicationStateManager::onOpenChatList(const QString &userId)
{
    if (m_currentState == m_splashScreenState || m_currentState == m_accountSelectionState ||
            m_currentState == m_signUpState || m_currentState == m_downloadKeyState) {
        m_chatListState->setUserId(userId);
        emit chatListRequested(QPrivateSignal());
    }
}

void ApplicationStateManager::onOpenPreview(const QUrl &url)
{
    if (m_currentState == m_chatState) {
        m_attachmentPreviewState->setUrl(url);
        emit openPreviewRequested(QPrivateSignal());
    }
}

void ApplicationStateManager::onOpenDownloadKey(const QString &userId)
{
    if (m_currentState == m_signInAsState) {
        m_downloadKeyState->setUserId(userId);
        emit keyDownloadRequested(QPrivateSignal());
    }
}

void ApplicationStateManager::onOpenAccountSettings(const QString &userId)
{
    if (m_currentState == m_chatListState) {
        m_accountSettingsState->setUserId(userId);
        emit openAccountSettingsRequested(QPrivateSignal());
    }
}

void ApplicationStateManager::onOpenBackupKey(const QString &userId)
{
    if (m_currentState == m_accountSettingsState) {
        m_backupKeyState->setUserId(userId);
        emit openBackupKeyRequested(QPrivateSignal());
    }
}

void ApplicationStateManager::onSignInUsernameValidated(const QString &userId)
{
    if (m_currentState == m_signInUsernameState) {
        m_signInAsState->setUserId(userId);
        emit signInAsRequested(QPrivateSignal());
    }
}
