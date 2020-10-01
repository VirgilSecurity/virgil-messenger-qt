import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0

import "./base"
import "./theme"
import "./components"
import "./helpers/login.js" as LoginLogic

Control {
    id: mainView

    property var chatListPushed: false

    RowLayout {
        anchors {
            fill: parent
            bottomMargin: logControl.visible ? logControl.height : 0
        }
        spacing: 0
        clip: logControl.visible

        ServersPanel {
            id: serversPanel
            visible: stackView.currentItem && typeof(stackView.currentItem.showServersPanel) !== "undefined" && stackView.currentItem.showServersPanel
            z: 2
            Layout.preferredWidth: 60
            Layout.fillHeight: true

            Action {
                text: qsTr("Settings")
                onTriggered: app.stateManager.setAccountSettingsState()
            }

            MenuSeparator {
                leftPadding: 20
            }

            Action {
                text: qsTr("Sign Out")
                onTriggered: app.stateManager.setSignOutState()
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

    Component.onCompleted: {
        app.stateManager.splashScreenState.entered.connect(pushSplashScreenPage)
        app.stateManager.accountSelectionState.entered.connect(pushAccountSelectionPage)
        app.stateManager.chatListState.entered.connect(pushChatListPage)
        app.stateManager.accountSettingsState.entered.connect(pushAccountSettingsPage)
        app.stateManager.setPreviousState.connect(stackView.pop)
        if (logControl.visible) {
            logging.formattedMessageCreated.connect(logTextControl.append)
        }
    }

    // FIXME(fpohtmeh): refactor
    /*
    function disconnect() {
        var future = Messenger.disconnect()
        Future.onFinished(future, function(value) {
          console.log("Logout result: ", Future.result(future))
            stackView.clear()
            showAuth(true)
        })
    }
    */

    function pushSplashScreenPage() {
        stackView.push("./pages/SplashScreenPage.qml", StackView.Immediate)
    }

    function pushAccountSelectionPage(animate) {
        stackView.push("./pages/AccountSelectionPage.qml", animate ? StackView.Transition : StackView.Immediate)
    }

    function pushChatListPage() {
        if (!chatListPushed) {
            stackView.clear()
            stackView.push("./pages/ChatListPage.qml")
            chatListPushed = true
        }
    }

    function pushAccountSettingsPage() {
        navigateTo("AccountSettings", null, true, false)
    }

    // FIXME(fpohtmeh): refactor
    /*
    function showSignIn() {
        stackView.push("./pages/SignInPage.qml")
    }

    function showSignInAs(params) {
        stackView.push("./pages/SignInAsPage.qml", params)
    }

    function showDownloadKey(params) {
        stackView.push("./pages/DownloadKeyPage.qml", params)
    }

    function showRegister() {
        stackView.push("./pages/RegisterPage.qml")
    }

    function showBackupKey() {
        stackView.push("./pages/BackupKeyPage.qml")
    }

    function showChatWith(recipient, replace) {
        navigateTo("Chat", { recipient: recipient }, true, false, replace)
        Messenger.setCurrentRecipient(recipient)
    }

    function showAddPerson() {
        stackView.push("./pages/AddPersonPage.qml")
    }
    */

    function navigateTo(page, params, animate, clearHistory, replace) {
        const pageName = "%1Page".arg(page)
        if (clearHistory) {
            stackView.clear()
        }
        var push = replace ? stackView.replace : stackView.push
        const path = "./pages/%1.qml".arg(pageName)
        push(path, params, animate ? StackView.Transition : StackView.Immediate)
    }
}
