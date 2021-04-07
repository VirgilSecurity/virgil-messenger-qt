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

Self::ApplicationStateManager(Messenger *messenger, Controllers *controllers, Models *models, Validator *validator,
                              QObject *parent)
    : QStateMachine(parent),
      m_messenger(messenger),
      m_controllers(controllers),
      m_validator(validator),
      m_settings(m_messenger->settings()),
      m_accountSelectionState(new AccountSelectionState(controllers->users(), this)),
      m_accountSettingsState(new AccountSettingsState(this)),
      m_addCloudFolderMembersState(
              new AddCloudFolderMembersState(controllers->cloudFiles(), models->discoveredContacts(), this)),
      m_addGroupChatMembersState(
              new AddGroupChatMembersState(controllers->chats(), models->discoveredContacts(), this)),
      m_attachmentPreviewState(new AttachmentPreviewState(this)),
      m_backupKeyState(new BackupKeyState(m_messenger, this)),
      m_editChatInfoState(new EditChatInfoState(m_messenger, controllers->chats(), this)),
      m_editProfileState(new EditProfileState(controllers->users(), this)),
      m_verifyProfileState(new VerifyProfileState(this)),
      m_chatInfoState(new ChatInfoState(controllers->chats(), this)),
      m_chatListState(new ChatListState(controllers, models->chats(), this)),
      m_chatState(new ChatState(controllers, m_messenger, this)),
      m_downloadKeyState(new DownloadKeyState(controllers->users(), this)),
      m_cloudFileListState(new CloudFileListState(messenger, controllers->cloudFiles(), this)),
      m_cloudFileSharingState(new CloudFileSharingState(controllers->cloudFiles(), this)),
      m_newChatState(new NewChatState(controllers->chats(), models->discoveredContacts(), this)),
      m_newCloudFolderMembersState(
              new NewCloudFolderMembersState(controllers->cloudFiles(), models->discoveredContacts(), this)),
      m_newGroupChatState(new NewGroupChatState(controllers->chats(), models->discoveredContacts(), this)),
      m_nameGroupChatState(new NameGroupChatState(this)),
      m_signInAsState(new SignInAsState(this)),
      m_signInUsernameState(new SignInUsernameState(controllers->users(), validator, this)),
      m_signUpState(new SignUpState(controllers->users(), validator, this)),
      m_startState(new StartState(controllers->users(), m_settings, this))
{
    registerStatesMetaTypes();
    addTransitions();
    setInitialState(m_startState);
    start();
}

Self::~ApplicationStateManager() { }

void Self::registerStatesMetaTypes()
{
    // Qt requires registering to avoid namespace issues
    qRegisterMetaType<AccountSelectionState *>("AccountSelectionState*");
    qRegisterMetaType<AccountSettingsState *>("AccountSettingsState*");
    qRegisterMetaType<AddCloudFolderMembersState *>("AddCloudFolderMembersState*");
    qRegisterMetaType<AddGroupChatMembersState *>("AddGroupChatMembersState*");
    qRegisterMetaType<AttachmentPreviewState *>("AttachmentPreviewState*");
    qRegisterMetaType<BackupKeyState *>("BackupKeyState*");
    qRegisterMetaType<EditChatInfoState *>("EditChatInfoState*");
    qRegisterMetaType<EditProfileState *>("EditProfileState*");
    qRegisterMetaType<VerifyProfileState *>("VerifyProfileState*");
    qRegisterMetaType<ChatInfoState *>("ChatInfoState*");
    qRegisterMetaType<ChatListState *>("ChatListState*");
    qRegisterMetaType<ChatState *>("ChatState*");
    qRegisterMetaType<DownloadKeyState *>("DownloadKeyState*");
    qRegisterMetaType<CloudFileListState *>("CloudFileListState*");
    qRegisterMetaType<CloudFileSharingState *>("CloudFileSharingState*");
    qRegisterMetaType<NewChatState *>("NewChatState*");
    qRegisterMetaType<NewCloudFolderMembersState *>("NewCloudFolderMembersState*");
    qRegisterMetaType<NewGroupChatState *>("NewGroupChatState*");
    qRegisterMetaType<NameGroupChatState *>("NameGroupChatState*");
    qRegisterMetaType<SignInAsState *>("SignInAsState*");
    qRegisterMetaType<SignInUsernameState *>("SignInUsernameState*");
    qRegisterMetaType<SignUpState *>("SignUpState*");
    qRegisterMetaType<StartState *>("StartState*");
}

void Self::addTransitions()
{
    const auto states = findChildren<QState *>();
    for (auto state : states) {
        connect(state, &QState::entered, this, std::bind(&Self::setCurrentState, this, state));
        connect(state, &QState::exited, this, std::bind(&Self::setPreviousState, this, state));
    }

    auto users = m_controllers->users();
    auto chats = m_controllers->chats();

    // NOTE: Queued connection is a workaround for working state transition
    connect(this, &Self::openChatList, this, std::bind(&Self::chatListRequested, this, QPrivateSignal()),
            Qt::QueuedConnection);
    connect(this, &Self::openCloudFileList, this, std::bind(&Self::cloudFileListRequested, this, QPrivateSignal()),
            Qt::QueuedConnection);

    m_startState->addTransition(m_startState, &StartState::chatListRequested, m_chatListState);
    m_startState->addTransition(m_startState, &StartState::accountSelectionRequested, m_accountSelectionState);

    m_accountSelectionState->addTransition(m_accountSelectionState, &AccountSelectionState::chatListRequested,
                                           m_chatListState);
    addTwoSideTransition(m_accountSelectionState, m_accountSelectionState,
                         &AccountSelectionState::requestSignInUsername, m_signInUsernameState);
    addTwoSideTransition(m_accountSelectionState, m_accountSelectionState, &AccountSelectionState::requestSignUp,
                         m_signUpState);

    m_chatListState->addTransition(users, &UsersController::signedOut, m_accountSelectionState);
    m_chatListState->addTransition(users, &UsersController::signInErrorOccured, m_accountSelectionState);
    addTwoSideTransition(m_chatListState, users, &UsersController::accountSettingsRequested, m_accountSettingsState);
    addTwoSideTransition(m_chatListState, m_chatListState, &ChatListState::requestNewChat, m_newChatState);
    addTwoSideTransition(m_chatListState, m_chatListState, &ChatListState::requestNewGroupChat, m_nameGroupChatState);
    addTwoSideTransition(m_chatListState, chats, &ChatsController::chatOpened, m_chatState);
    m_chatListState->addTransition(this, &Self::cloudFileListRequested, m_cloudFileListState);

    addTwoSideTransition(m_nameGroupChatState, m_nameGroupChatState, &NameGroupChatState::groupNamed,
                         m_newGroupChatState);
    connect(m_nameGroupChatState, &NameGroupChatState::groupNamed, m_newGroupChatState, &NewGroupChatState::setName);

    m_newGroupChatState->addTransition(chats, &ChatsController::chatOpened, m_chatState);

    addTwoSideTransition(m_cloudFileListState, users, &UsersController::accountSettingsRequested,
                         m_accountSettingsState);
    addTwoSideTransition(m_cloudFileListState, m_cloudFileListState, &CloudFileListState::requestNewSharedFolder,
                         m_newCloudFolderMembersState);
    addTwoSideTransition(m_cloudFileListState, m_cloudFileListState, &CloudFileListState::requestSharingInfo,
                         m_cloudFileSharingState);
    connect(m_cloudFileListState, &CloudFileListState::requestNewSharedFolder, m_newCloudFolderMembersState,
            &NewCloudFolderMembersState::setName);
    m_cloudFileListState->addTransition(users, &UsersController::signedOut, m_accountSelectionState);
    m_cloudFileListState->addTransition(this, &Self::chatListRequested, m_chatListState);

    addTwoSideTransition(m_cloudFileSharingState, m_cloudFileSharingState, &CloudFileSharingState::addMembersRequested,
                         m_addCloudFolderMembersState);

    connect(m_newCloudFolderMembersState, &NewCloudFolderMembersState::contactsSelected, this,
            &ApplicationStateManager::goBack);
    connect(m_addCloudFolderMembersState, &AddCloudFolderMembersState::contactsSelected, this,
            &ApplicationStateManager::goBack);

    addTwoSideTransition(m_accountSettingsState, m_accountSettingsState, &AccountSettingsState::requestBackupKey,
                         m_backupKeyState);
    m_accountSettingsState->addTransition(users, &UsersController::signedOut, m_accountSelectionState);
    addTwoSideTransition(m_accountSettingsState, m_accountSettingsState, &AccountSettingsState::editProfile,
                         m_editProfileState);

    addTwoSideTransition(m_editProfileState, m_editProfileState, &EditProfileState::verify, m_verifyProfileState);
    connect(m_editProfileState, &EditProfileState::verify, m_verifyProfileState, &VerifyProfileState::setCodeType);
    connect(m_verifyProfileState, &VerifyProfileState::verificationFinished, m_editProfileState,
            &EditProfileState::processVerificationResponse);

    m_newChatState->addTransition(chats, &ChatsController::chatOpened, m_chatState);
    m_newChatState->addTransition(users, &UsersController::accountSettingsRequested, m_accountSettingsState);
    m_newChatState->addTransition(users, &UsersController::signedOut, m_accountSelectionState);

    addTwoSideTransition(m_chatState, m_chatState, &ChatState::requestPreview, m_attachmentPreviewState);
    connect(m_chatState, &ChatState::requestPreview, m_attachmentPreviewState, &AttachmentPreviewState::setUrl);
    addTwoSideTransition(m_chatState, m_chatState, &ChatState::requestInfo, m_chatInfoState);
    connect(chats, &ChatsController::groupInvitationRejected, this, &ApplicationStateManager::goBack);

    addTwoSideTransition(m_chatInfoState, m_chatInfoState, &ChatInfoState::addMembersRequested,
                         m_addGroupChatMembersState);
    addTwoSideTransition(m_chatInfoState, m_chatInfoState, &ChatInfoState::editRequested, m_editChatInfoState);
    connect(m_addGroupChatMembersState, &AddGroupChatMembersState::contactsSelected, this,
            &ApplicationStateManager::goBack);

    connect(m_editChatInfoState, &EditChatInfoState::editingFinished, this, &Self::goBack);

    m_signUpState->addTransition(users, &UsersController::signedIn, m_chatListState);

    addTwoSideTransition(m_signInUsernameState, m_signInUsernameState, &SignInUsernameState::validated,
                         m_signInAsState);

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
