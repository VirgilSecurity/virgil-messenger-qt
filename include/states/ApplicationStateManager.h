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

#ifndef VM_APPLICATIONSTATEMANAGER_H
#define VM_APPLICATIONSTATEMANAGER_H

#include <QStateMachine>

#include "BackupKeyState.h"
#include "ChatState.h"
#include "CloudFileListState.h"
#include "DownloadKeyState.h"
#include "EditChatInfoState.h"
#include "EditProfileState.h"
#include "GroupChatMembersState.h"
#include "NewChatState.h"
#include "RestoreAccountUsernameState.h"
#include "CloudFolderMembersState.h"
#include "SignUpState.h"
#include "StartState.h"
#include "VerifyProfileState.h"

class Settings;

namespace vm {
class Controllers;
class Messenger;
class Models;
class Validator;

class ApplicationStateManager : public QStateMachine
{
    Q_OBJECT
    Q_PROPERTY(CloudFolderMembersState *cloudFolderMembersState MEMBER m_cloudFolderMembersState CONSTANT)
    Q_PROPERTY(BackupKeyState *backupKeyState MEMBER m_backupKeyState CONSTANT)
    Q_PROPERTY(EditChatInfoState *editChatInfoState MEMBER m_editChatInfoState CONSTANT)
    Q_PROPERTY(EditProfileState *editProfileState MEMBER m_editProfileState CONSTANT)
    Q_PROPERTY(VerifyProfileState *verifyProfileState MEMBER m_verifyProfileState CONSTANT)
    Q_PROPERTY(ChatState *chatState MEMBER m_chatState CONSTANT)
    Q_PROPERTY(DownloadKeyState *downloadKeyState MEMBER m_downloadKeyState CONSTANT)
    Q_PROPERTY(CloudFileListState *cloudFileListState MEMBER m_cloudFileListState CONSTANT)
    Q_PROPERTY(NewChatState *newChatState MEMBER m_newChatState CONSTANT)
    Q_PROPERTY(GroupChatMembersState *groupChatMembersState MEMBER m_groupChatMembersState CONSTANT)
    Q_PROPERTY(RestoreAccountUsernameState *restoreAccountUsernameState MEMBER m_restoreAccountUsernameState CONSTANT)
    Q_PROPERTY(SignUpState *signUpState MEMBER m_signUpState CONSTANT)
    Q_PROPERTY(StartState *startState MEMBER m_startState CONSTANT)

public:
    ApplicationStateManager(Messenger *messenger, Controllers *controllers, Models *models, Validator *validator,
                            QObject *parent);
    ~ApplicationStateManager() override;

private:
    Messenger *m_messenger;
    Controllers *m_controllers;
    Validator *m_validator;
    Settings *m_settings;

    CloudFolderMembersState *m_cloudFolderMembersState;
    BackupKeyState *m_backupKeyState;
    EditChatInfoState *m_editChatInfoState;
    EditProfileState *m_editProfileState;
    VerifyProfileState *m_verifyProfileState;
    ChatState *m_chatState;
    DownloadKeyState *m_downloadKeyState;
    CloudFileListState *m_cloudFileListState;
    NewChatState *m_newChatState;
    GroupChatMembersState *m_groupChatMembersState;
    RestoreAccountUsernameState *m_restoreAccountUsernameState;
    SignUpState *m_signUpState;
    StartState *m_startState;
};
} // namespace vm

#endif // VM_APPLICATIONSTATEMANAGER_H
