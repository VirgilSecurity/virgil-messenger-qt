import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0

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
                onTriggered: manager.openAccountSettings()
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

    TextScrollView {
        id: logControl
        anchors {
            topMargin: 0.75 * mainView.height
            fill: parent
        }
        visible: settings.devMode

        TextArea {
            id: logTextControl
            width: mainView.width
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            readOnly: true
            font.pointSize: Platform.isMobile ? 12 : 9
            selectByMouse: Platform.isDesktop
        }
    }

    Button {
        id: clearLogButton
        visible: logControl.visible
        anchors.right: logControl.right
        anchors.top: logControl.top
        text: "x"
        width: 20
        height: width
        onClicked: logTextControl.clear()
    }

    QtObject {
        id: d

        function goBack() {
            if (attachmentPreview.visible) {
                attachmentPreview.visible = false
            }
            else {
                stackView.pop()
            }
        }

        function openSplashScreenPage() {
            stackView.push("./pages/SplashScreenPage.qml", StackView.Immediate)
        }

        function openAccountSelectionPage() {
            if ([manager.signUpState, manager.signInState].includes(manager.previousState)) {
                return
            }
            stackView.push("./pages/AccountSelectionPage.qml", StackView.Immediate)
        }

        function openChatListPage() {
            if ([manager.splashScreenState, manager.accountSelectionState, manager.signUpState].includes(manager.previousState)) {
                stackView.clear()
                stackView.push("./pages/ChatListPage.qml")
            }
        }

        function openAccountSettingsPage() {
            if (manager.previousState !== manager.chatListState) {
                return
            }
            stackView.push("./pages/AccountSettingsPage.qml", StackView.Transition)
        }

        function openAddNewChatPage() {
            stackView.push("./pages/NewChatPage.qml")
        }

        function openChatPage() {
            if (manager.previousState === manager.attachmentPreviewState) {
                return
            }
            const replace = manager.previousState === manager.newChatState
            var push = replace ? stackView.replace : stackView.push
            stackView.push("./pages/ChatPage.qml", StackView.Transition)
        }

        function showAttachmentPreview() {
            attachmentPreview.visible = true
        }

        function openBackupKeyPage() {
            if (manager.previousState !== manager.accountSettingsState) {
                return
            }
            stackView.push("./pages/BackupKeyPage.qml")
        }

        function openSignInAsPage() {
            stackView.push("./pages/SignInAsPage.qml")
        }

        function openSignInPage() {
            if (manager.previousState === manager.accountSelectionState) {
                stackView.push("./pages/SignInPage.qml")
            }
        }

        function openSignUpPage() {
            if (manager.previousState === manager.accountSelectionState) {
                stackView.push("./pages/SignUpPage.qml")
            }
        }

        // FIXME(fpohtmeh): refactor
        /*
        function showDownloadKey(params) {
            stackView.push("./pages/DownloadKeyPage.qml", params)
        }
        */
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
        manager.signInState.entered.connect(d.openSignInPage)
        manager.signUpState.entered.connect(d.openSignUpPage)

        if (logControl.visible) {
            logging.formattedMessageCreated.connect(logTextControl.append)
        }
    }
}
