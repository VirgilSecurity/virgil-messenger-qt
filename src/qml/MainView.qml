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

    RowLayout {
        anchors {
            fill: parent
            bottomMargin: logControl.visible ? logControl.height : 0
        }
        spacing: 0
        clip: logControl.visible

        ServersPanel {
            id: serversPanel
            visible: [manager.chatListState, manager.newChatState, manager.accountSettingsState].includes(manager.currentState)
            z: 2
            Layout.preferredWidth: 60
            Layout.fillHeight: true

            Action {
                text: qsTr("Settings")
                onTriggered: manager.openAccountSettings(Messenger.currentUser)
            }

            MenuSeparator {
                leftPadding: 20
            }

            Action {
                text: qsTr("Sign Out")
                onTriggered: manager.signOut()
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
            if ([manager.splashScreenState, manager.accountSelectionState, manager.signUpState].includes(manager.previousState)) {
                stackView.clear()
                stackView.push(page("ChatList"))
            }
        }

        function openAccountSettingsPage() {
            if (manager.previousState !== manager.chatListState) {
                return
            }
            stackView.push(page("AccountSettings"), StackView.Transition)
        }

        function openAddNewChatPage() {
            stackView.push(page("NewChat"))
        }

        function openChatPage() {
            if (manager.previousState === manager.attachmentPreviewState) {
                return
            }
            const replace = [manager.newChatState, manager.downloadKeyState].includes(manager.previousState)
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
        manager.chatState.entered.connect(d.openChatPage)
        manager.attachmentPreviewState.entered.connect(d.showAttachmentPreview)
        manager.backupKeyState.entered.connect(d.openBackupKeyPage)
        manager.signInAsState.entered.connect(d.openSignInAsPage)
        manager.signInUsernameState.entered.connect(d.openSignInUsernamePage)
        manager.signUpState.entered.connect(d.openSignUpPage)
        manager.downloadKeyState.entered.connect(d.openDownloadKeyPage)
    }
}
