import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "./base"
import "./theme"
import "./components"

Control {
    id: mainView

    property var attachmentPreview: undefined
    readonly property var manager: app.stateManager

    ColumnLayout {
        anchors {
            fill: parent
            bottomMargin: keyboardHandler.keyboardHeight
        }
        spacing: 0

        StackView {
            id: stackView
            background: Rectangle {
                color: Theme.contactsBackgroundColor
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        LogControl {
            visible: settings.devMode
            Layout.fillWidth: true
            Layout.preferredHeight: 0.25 * mainView.height
        }
    }

    QtObject {
        id: d

        function page(name) {
            return "./pages/%1Page.qml".arg(name)
        }

        function goBack() {
            if (attachmentPreview.visible) {
                attachmentPreview.visible = false
            }
            else if (manager.currentState === manager.cloudFileListState) {
                controllers.cloudFiles.switchToParentFolder()
            }
            else {
                stackView.pop()
            }
        }

        function openSplashScreenPage() {
            stackView.push(page("SplashScreen"), StackView.Immediate)
        }

        function openAccountSelectionPage() {
            if ([manager.signUpState, manager.signInState].includes(manager.previousState)) {
                return
            }
            stackView.push(page("AccountSelection"), StackView.Immediate)
        }

        function openMainPage() {
            if ([manager.splashScreenState, manager.accountSelectionState,
                 manager.signUpState, manager.downloadKeyState, manager.cloudFileListState].includes(manager.previousState)) {
                stackView.clear()
                stackView.push(page("Main"))
            }
        }

        function openAccountSettingsPage() {
            if (![manager.chatListState, manager.cloudFileListState, manager.newChatState].includes(manager.previousState)) {
                return
            }
            stackView.push(page("AccountSettings"), StackView.Transition)
        }

        function openAddNewChatPage() {
            if (![manager.chatListState, manager.cloudFileListState, manager.newChatState].includes(manager.previousState)) {
                return
            }
            stackView.push(page("NewChat"))
        }

        function openAddNewGroupChatPage() {
            if (![manager.chatListState, manager.cloudFileListState, manager.newChatState].includes(manager.previousState)) {
                return
            }
            stackView.push(page("NewGroupChat"))
        }

        function openNameGroupChatPage() {
            stackView.push(page("NameGroupChat"))
        }

        function openChatPage() {
            if ([manager.attachmentPreviewState, manager.chatInfoState].includes(manager.previousState)) {
                return
            }
            const replace = [manager.newChatState, manager.nameGroupChatState, manager.downloadKeyState].includes(manager.previousState)
            if (manager.previousState === manager.nameGroupChatState) {
                stackView.pop()
            }
            var push = replace ? stackView.replace : stackView.push
            push(page("Chat"), StackView.Transition)
        }

        function openChatInfoPage() {
            if (manager.previousState !== manager.addGroupChatMembersState) {
                stackView.push(page("ChatInfo"))
            }
        }

        function openAddGroupChatMembersPage() {
            stackView.push(page("AddGroupChatMembers"))
        }

        function showAttachmentPreview() {
            attachmentPreview.visible = true
        }

        function openBackupKeyPage() {
            if (manager.previousState !== manager.accountSettingsState) {
                return
            }
            stackView.push(page("BackupKey"))
        }

        function openEditProfilePage() {
            if (manager.previousState !== manager.accountSettingsState) {
                return
            }
            stackView.push(page("EditProfile"))
        }

        function openVerifyProfilePage() {
            if (manager.previousState !== manager.editProfileState) {
                return
            }
            stackView.push(page("VerifyProfile"))
        }

        function openSignInAsPage() {
            if (manager.previousState !== manager.downloadKeyState) {
                stackView.push(page("SignInAs"))
            }
        }

        function openSignInUsernamePage() {
            if (manager.previousState === manager.accountSelectionState) {
                stackView.push(page("SignInUsername"))
            }
        }

        function openSignUpPage() {
            if (manager.previousState === manager.accountSelectionState) {
                stackView.push(page("SignUp"))
            }
        }

        function openDownloadKeyPage() {
            stackView.push(page("DownloadKey"))
        }

        function openNewCloudFolderMembersPage() {
            stackView.push(page("NewCloudFolderMembers"))
        }

        function openAddCloudFolderMembersPage() {
            stackView.push(page("AddCloudFolderMembers"))
        }

        function openCloudFileSharingPage() {
            if (manager.previousState !== manager.addCloudFolderMembersState) {
                stackView.push(page("CloudFileSharing"))
            }
        }
    }

    Component.onCompleted: {
        manager.goBack.connect(d.goBack)
        manager.splashScreenState.entered.connect(d.openSplashScreenPage)
        manager.accountSelectionState.entered.connect(d.openAccountSelectionPage)
        manager.chatListState.entered.connect(d.openMainPage)
        manager.accountSettingsState.entered.connect(d.openAccountSettingsPage)
        manager.newChatState.entered.connect(d.openAddNewChatPage)
        manager.newGroupChatState.entered.connect(d.openAddNewGroupChatPage)
        manager.nameGroupChatState.entered.connect(d.openNameGroupChatPage)
        manager.chatState.entered.connect(d.openChatPage)
        manager.chatInfoState.entered.connect(d.openChatInfoPage)
        manager.addGroupChatMembersState.entered.connect(d.openAddGroupChatMembersPage)
        manager.attachmentPreviewState.entered.connect(d.showAttachmentPreview)
        manager.backupKeyState.entered.connect(d.openBackupKeyPage)
        manager.editProfileState.entered.connect(d.openEditProfilePage)
        manager.verifyProfileState.entered.connect(d.openVerifyProfilePage)
        manager.signInAsState.entered.connect(d.openSignInAsPage)
        manager.signInUsernameState.entered.connect(d.openSignInUsernamePage)
        manager.signUpState.entered.connect(d.openSignUpPage)
        manager.downloadKeyState.entered.connect(d.openDownloadKeyPage)
        manager.newCloudFolderMembersState.entered.connect(d.openNewCloudFolderMembersPage)
        manager.addCloudFolderMembersState.entered.connect(d.openAddCloudFolderMembersPage)
        manager.cloudFileSharingState.entered.connect(d.openCloudFileSharingPage)
    }
}
