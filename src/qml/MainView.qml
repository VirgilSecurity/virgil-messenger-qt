import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "./base"
import "./theme"
import "./components"

Control {
    id: mainView

    property var attachmentPreview: undefined

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
        readonly property var manager: app.stateManager

        function page(name) {
            return "./pages/%1Page.qml".arg(name)
        }

        function goBack() {
            if (attachmentPreview.visible) {
                attachmentPreview.visible = false
            }
            else if (manager.currentState === manager.cloudFileListState) {
                var selection = models.cloudFiles.selection
                if (selection.hasSelection) {
                    selection.clear()
                }
                else {
                    controllers.cloudFiles.switchToParentFolder()
                }
            }
            else {
                stackView.pop()
            }
        }

        function openAccountSelectionPage() {
            if (manager.signUpState === manager.previousState) {
                return
            }
            stackView.push(page("AccountSelection"), StackView.Immediate)
        }

        function openMainPage() {
            if ([manager.startState, manager.accountSelectionState,
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
            stackView.push(page("NewGroupChat"))
        }

        function openNameGroupChatPage() {
            if (![manager.chatListState, manager.cloudFileListState].includes(manager.previousState)) {
                return
            }
            stackView.push(page("NameGroupChat"))
        }

        function openChatPage() {
            // TODO(fpohtmeh): improve this code
            if ([manager.newChatState, manager.newGroupChatState, manager.downloadKeyState].includes(manager.previousState)) {
                stackView.pop()
            }
            if (manager.previousState === manager.newGroupChatState) {
                stackView.pop()
            }
        }

        function openChatInfoPage() {
            if (![manager.addGroupChatMembersState, manager.editChatInfoState].includes(manager.previousState)) {
                stackView.push(page("ChatInfo"))
            }
        }

        function openEditChatInfoPage() {
            stackView.push(page("EditChatInfo"))
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

        function setupConnections() {
            manager.goBack.connect(goBack)
            manager.accountSelectionState.entered.connect(openAccountSelectionPage)
            manager.chatListState.entered.connect(openMainPage)
            manager.chatState.entered.connect(openChatPage)
            manager.accountSettingsState.entered.connect(openAccountSettingsPage)
            manager.newChatState.entered.connect(openAddNewChatPage)
            manager.newGroupChatState.entered.connect(openAddNewGroupChatPage)
            manager.nameGroupChatState.entered.connect(openNameGroupChatPage)
            manager.chatInfoState.entered.connect(openChatInfoPage)
            manager.editChatInfoState.entered.connect(openEditChatInfoPage)
            manager.addGroupChatMembersState.entered.connect(openAddGroupChatMembersPage)
            manager.attachmentPreviewState.entered.connect(showAttachmentPreview)
            manager.backupKeyState.entered.connect(openBackupKeyPage)
            manager.editProfileState.entered.connect(openEditProfilePage)
            manager.verifyProfileState.entered.connect(openVerifyProfilePage)
            manager.signInAsState.entered.connect(openSignInAsPage)
            manager.signInUsernameState.entered.connect(openSignInUsernamePage)
            manager.signUpState.entered.connect(openSignUpPage)
            manager.downloadKeyState.entered.connect(openDownloadKeyPage)
            manager.newCloudFolderMembersState.entered.connect(openNewCloudFolderMembersPage)
            manager.addCloudFolderMembersState.entered.connect(openAddCloudFolderMembersPage)
            manager.cloudFileSharingState.entered.connect(openCloudFileSharingPage)
        }
    }

    Component.onCompleted: d.setupConnections()
}
