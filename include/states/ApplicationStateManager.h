﻿//  Copyright (C) 2015-2020 Virgil Security, Inc.
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
#include "AttachmentPreviewState.h"
#include "BackupKeyState.h"
#include "ChatListState.h"
#include "ChatState.h"
#include "NewChatState.h"
#include "SignInAsState.h"
#include "SignInState.h"
#include "SignUpState.h"
#include "SplashScreenState.h"
#include "StartState.h"
#include "VSQCommon.h"

Q_DECLARE_LOGGING_CATEGORY(lcAppState);

namespace VSQ
{
class ApplicationStateManager : public QStateMachine
{
    Q_OBJECT
    Q_PROPERTY(AccountSelectionState *accountSelectionState MEMBER m_accountSelectionState CONSTANT)
    Q_PROPERTY(AccountSettingsState *accountSettingsState MEMBER m_accountSettingsState CONSTANT)
    Q_PROPERTY(AttachmentPreviewState *attachmentPreviewState MEMBER m_attachmentPreviewState CONSTANT)
    Q_PROPERTY(BackupKeyState *backupKeyState MEMBER m_backupKeyState CONSTANT)
    Q_PROPERTY(ChatListState *chatListState MEMBER m_chatListState CONSTANT)
    Q_PROPERTY(ChatState *chatState MEMBER m_chatState CONSTANT)
    Q_PROPERTY(NewChatState *newChatState MEMBER m_newChatState CONSTANT)
    Q_PROPERTY(SignInAsState *signInAsState MEMBER m_signInAsState CONSTANT)
    Q_PROPERTY(SignInState *signInState MEMBER m_signInState CONSTANT)
    Q_PROPERTY(SignUpState *signUpState MEMBER m_signUpState CONSTANT)
    Q_PROPERTY(SplashScreenState *splashScreenState MEMBER m_splashScreenState CONSTANT)
    Q_PROPERTY(StartState *startState MEMBER m_startState CONSTANT)

    Q_PROPERTY(QState *currentState MEMBER m_currentState NOTIFY currentStateChanged)
    Q_PROPERTY(QState *previousState MEMBER m_previousState NOTIFY previousStateChanged)

public:
    explicit ApplicationStateManager(VSQMessenger *messenger, VSQSettings *settings, QObject *parent);
    ~ApplicationStateManager() override;

signals:
    void setUiState();
    void goBack();
    void signIn(const QString &userId);
    void signUp(const QString &userId);
    void signOut();
    void sendMessage(const QString &to, const QString &message, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType);
    void openSignIn();
    void openSignInAs(const QString &userId);
    void openSignUp();
    void openAddContact();
    void addContact(const QString &contactId);
    void openChat(const QString &contactId);
    void openAccountSettings();
    void openBackupKey();
    void backupKey(const QString &password, const QString &confirmedPassword);

    void currentStateChanged(QState *);
    void previousStateChanged(QState *);

    void splashScreenRequested(QPrivateSignal);
    void chatRequested(QPrivateSignal);
    void openPreviewRequested(QPrivateSignal);

private:
    void registerStatesMetaTypes();
    void setupConnections();
    void addTransitions();

    template <typename Func>
    void addTwoSideTransition(QState *source, const typename QtPrivate::FunctionPointer<Func>::Object *obj, Func signal, QState *target)
    {
        source->addTransition(obj, signal, target);
        target->addTransition(this, &ApplicationStateManager::goBack, source);
    }

    void setCurrentState(QState *state);
    void setPreviousState(QState *state);
    void onSignIn(const QString &userId);
    void onSignUp(const QString &userId);
    void onOpenChat(const QString &contactId);
    void onOpenPreview(const QUrl &url);

    VSQMessenger *m_messenger;
    VSQSettings *m_settings;

    AccountSelectionState *m_accountSelectionState;
    AccountSettingsState *m_accountSettingsState;
    AttachmentPreviewState *m_attachmentPreviewState;
    BackupKeyState *m_backupKeyState;
    ChatListState *m_chatListState;
    ChatState *m_chatState;
    NewChatState *m_newChatState;
    SignInAsState *m_signInAsState;
    SignInState *m_signInState;
    SignUpState *m_signUpState;
    SplashScreenState *m_splashScreenState;
    StartState *m_startState;

    QState *m_currentState = nullptr;
    QState *m_previousState = nullptr;
};
}

#endif // VSQ_APPLICATIONSTATEMANAGER_H
