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

#ifndef VSQ_APPLICATIONSTATEMANAGER_H
#define VSQ_APPLICATIONSTATEMANAGER_H

#include <QStateMachine>
#include <QTimer>

#include "AccountSelectionState.h"
#include "AccountSettingsState.h"
#include "BackupKeyState.h"
#include "ChatListState.h"
#include "ChatState.h"
#include "NewChatState.h"
#include "SplashScreenState.h"
#include "StartState.h"

namespace VSQ
{
class ApplicationStateManager : public QStateMachine
{
    Q_OBJECT
    Q_PROPERTY(StartState *startState MEMBER m_startState CONSTANT)
    Q_PROPERTY(SplashScreenState *splashScreenState MEMBER m_splashScreenState CONSTANT)
    Q_PROPERTY(AccountSelectionState *accountSelectionState MEMBER m_accountSelectionState CONSTANT)
    Q_PROPERTY(ChatListState *chatListState MEMBER m_chatListState CONSTANT)
    Q_PROPERTY(ChatState *chatState MEMBER m_chatState CONSTANT)
    Q_PROPERTY(NewChatState *newChatState MEMBER m_newChatState CONSTANT)
    Q_PROPERTY(AccountSettingsState *accountSettingsState MEMBER m_accountSettingsState CONSTANT)
    Q_PROPERTY(BackupKeyState *backupKeyState MEMBER m_backupKeyState CONSTANT)

public:
    explicit ApplicationStateManager(VSQMessenger *messenger, VSQSettings *settings, QObject *parent);
    ~ApplicationStateManager() override;

signals:
    void setUiState();
    void setSignOutState();
    void setAccountSettingsState();
    void setPreviousState();

    void splashScreenRequested(QPrivateSignal);

private:
    void registerStatesMetaTypes();
    void setupConnections();
    void addTransitions();

    VSQMessenger *m_messenger;
    VSQSettings *m_settings;

    StartState *m_startState;
    SplashScreenState *m_splashScreenState;
    AccountSelectionState *m_accountSelectionState;
    ChatListState *m_chatListState;
    ChatState *m_chatState;
    NewChatState *m_newChatState;
    AccountSettingsState *m_accountSettingsState;
    BackupKeyState *m_backupKeyState;
};
}

#endif // VSQ_APPLICATIONSTATEMANAGER_H
