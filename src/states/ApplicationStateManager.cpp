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
#include "Models.h"
#include "ChatsController.h"
#include "CloudFilesController.h"
#include "Controllers.h"
#include "UsersController.h"

#include <QLoggingCategory>

using namespace vm;
using Self = ApplicationStateManager;

Q_LOGGING_CATEGORY(lcAppState, "app-state");

Self::ApplicationStateManager(Messenger *messenger, Controllers *controllers, Models *models, Validator *validator,
                              QObject *parent)
    : QStateMachine(parent),
      m_messenger(messenger),
      m_controllers(controllers),
      m_validator(validator),
      m_settings(m_messenger->settings()),
      m_emptyState(new QState(this)),
      m_cloudFolderMembersState(
              new CloudFolderMembersState(controllers->cloudFiles(), models->discoveredContacts(), this)),
      m_backupKeyState(new BackupKeyState(m_messenger, this)),
      m_editChatInfoState(new EditChatInfoState(m_messenger, controllers->chats(), this)),
      m_editProfileState(new EditProfileState(controllers->users(), this)),
      m_verifyProfileState(new VerifyProfileState(this)),
      m_chatState(new ChatState(controllers, m_messenger, this)),
      m_downloadKeyState(new DownloadKeyState(controllers->users(), this)),
      m_cloudFileListState(new CloudFileListState(messenger, controllers->cloudFiles(), this)),
      m_newChatState(new NewChatState(controllers->chats(), models->discoveredContacts(), this)),
      m_groupChatMembersState(new GroupChatMembersState(controllers->chats(), models->discoveredContacts(), this)),
      m_restoreAccountUsernameState(new RestoreAccountUsernameState(validator, this)),
      m_signUpState(new SignUpState(controllers->users(), validator, this)),
      m_startState(new StartState(controllers->users(), m_settings, this))
{
    addConnections();
    setInitialState(m_startState);
    start();
}

Self::~ApplicationStateManager() { }

void Self::enterState(QState *state)
{
    const auto newState = state ? state : m_emptyState;
    if (m_currentState == newState) {
        return;
    }
    if (m_transition) {
        delete m_transition;
    }
    m_transition = m_currentState->addTransition(this, &Self::enterStateCall, newState);
    emit enterStateCall(QPrivateSignal());
}

void Self::addConnections()
{
    const auto states = findChildren<QState *>();
    for (auto state : states) {
        connect(state, &QState::entered, this, std::bind(&Self::setCurrentState, this, state));
        connect(state, &QState::exited, this, std::bind(&Self::setPreviousState, this, state));
    }
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
