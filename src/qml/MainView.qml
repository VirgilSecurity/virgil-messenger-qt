import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "./base"
import "./theme"
import "./components"

Control {
    id: mainView

    property var attachmentPreview: undefined
    property int keyboardHeight: 0
    readonly property var manager: app.stateManager

    RowLayout {
        anchors {
            fill: parent
            bottomMargin: (logControl.visible ? logControl.height : 0) + keyboardHeight
        }
        spacing: 0
        clip: logControl.visible

        SidebarPanel {
            id: sideBar
            visible: [manager.chatListState, manager.cloudFileListState].includes(manager.currentState)
            z: 2
            Layout.preferredWidth: Theme.headerHeight
            Layout.fillHeight: true
            focus: true
            opacity: 0.99999 // Bug. If the transparency is set to 1, the images will disappear after stack.pop()

            Action {
                text: qsTr("Settings")
                onTriggered: controllers.users.requestAccountSettings(controllers.users.currentUsername)
            }

            ContextMenuSeparator {
            }

            Action {
                text: qsTr("Sign Out")
                onTriggered: controllers.users.signOut()
            }
        }

        StackView {
            id: stackView
            spacing: 0
            z: 1
            Layout.fillHeight: true
            Layout.fillWidth: true

            background: Rectangle {
                color: Theme.contactsBackgroundColor
            }
        }
    }

    UploadProgressBar {
        // TODO(fpohtmeh): move to main.qml
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        visible: manager.currentState === manager.cloudFileListState ? "opened" : "closed"
    }

    LogControl {
        id: logControl
        anchors {
            fill: parent
            topMargin: 0.75 * mainView.height
        }
        visible: settings.devMode
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
                controllers.cloudFiles.cdUp()
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

        function openChatListPage() {
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
            if (manager.previousState === manager.attachmentPreviewState) {
                return
            }
            const replace = [manager.newChatState, manager.nameGroupChatState, manager.downloadKeyState].includes(manager.previousState)
            if (manager.previousState === manager.nameGroupChatState) {
                stackView.pop()
            }
            var push = replace ? stackView.replace : stackView.push
            push(page("Chat"), StackView.Transition)
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
    }

    Component.onCompleted: {
        manager.goBack.connect(d.goBack)
        manager.splashScreenState.entered.connect(d.openSplashScreenPage)
        manager.accountSelectionState.entered.connect(d.openAccountSelectionPage)
        manager.chatListState.entered.connect(d.openChatListPage)
        manager.accountSettingsState.entered.connect(d.openAccountSettingsPage)
        manager.newChatState.entered.connect(d.openAddNewChatPage)
        manager.newGroupChatState.entered.connect(d.openAddNewGroupChatPage)
        manager.nameGroupChatState.entered.connect(d.openNameGroupChatPage)
        manager.chatState.entered.connect(d.openChatPage)
        manager.attachmentPreviewState.entered.connect(d.showAttachmentPreview)
        manager.backupKeyState.entered.connect(d.openBackupKeyPage)
        manager.editProfileState.entered.connect(d.openEditProfilePage)
        manager.verifyProfileState.entered.connect(d.openVerifyProfilePage)
        manager.signInAsState.entered.connect(d.openSignInAsPage)
        manager.signInUsernameState.entered.connect(d.openSignInUsernamePage)
        manager.signUpState.entered.connect(d.openSignUpPage)
        manager.downloadKeyState.entered.connect(d.openDownloadKeyPage)

        if (Platform.isIos) {
            app.keyboardEventFilter.keyboardRectangleChanged.connect(function(rect) {
                keyboardHeight = Math.max(0, root.height - rect.y)
            })
        }
    }
}
