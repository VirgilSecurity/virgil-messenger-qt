import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0

import "./theme"
import "./components"
import "./helpers/login.js" as LoginLogic

Control {
    id: mainView

    RowLayout {
        anchors {
            fill: parent
            bottomMargin: logControl.visible ? logControl.height : 0
        }
        spacing: 0

        ServersPanel {
            id: serversPanel
            visible: stackView.currentItem && typeof(stackView.currentItem.showServersPanel) !== "undefined" && stackView.currentItem.showServersPanel
            z: 2
            Layout.preferredWidth: 60
            Layout.fillHeight: true

            Action {
                text: qsTr("Settings")
                onTriggered: mainView.showAccountSettings()
            }

            MenuSeparator {
                leftPadding: 20
            }

            Action {
                text: "Sign Out"
                onTriggered: messenger.signOut()
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

    ScrollView {
        id: logControl
        anchors {
            topMargin: 0.75 * mainView.height
            fill: parent
        }
        visible: settings.devMode

        TextArea {
            id: logTextControl
            width: mainView.width
            wrapMode: Text.WordWrap
            readOnly: true
            font.pointSize: 9
        }
    }

    Component.onCompleted: {
        showSplashScreen()
        if (logControl.visible)
            logging.newMessage.connect(logTextControl.append)
    }

    function signIn(user) {
        if (LoginLogic.validateUser(user)) {
            var future = Messenger.signIn(user)
            Future.onFinished(future, function(value) {
              console.log("Log In result: ", Future.result(future))
            })

            stackView.clear()
            settings.lastSignedInUser = user
            showContacts()
        } else {
            window.showPopupError(qsTr("Incorrect User Name"))
        }
    }

    function disconnect() {
        var future = Messenger.disconnect()
        Future.onFinished(future, function(value) {
          console.log("Logout result: ", Future.result(future))
            stackView.clear()
            showAuth(true)
        })
    }

    // Navigation
    //
    // All the app navigation must be done throught executing
    // the functions below.

    function back() {
        stackView.pop()
        if (messenger.recipient) {
            messenger.recipient = ""
        }
    }

    function showSplashScreen(){
        stackView.push("./pages/SplashScreenPage.qml", StackView.Immediate)
    }

    function showAuth(animate) {
        if (animate) {
            stackView.push("./pages/AuthPage.qml")
            return
        }

        stackView.push("./pages/AuthPage.qml", StackView.Immediate)
    }

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

    function showChatWith(recipient) {
        navigateTo("Chat", { recipient: recipient }, true, false)
        messenger.recipient = recipient
    }

    function showContacts(clear) {
        if (clear) {
            stackView.clear()
        }

        stackView.push("./pages/ChatListPage.qml")
    }

    function showAccountSettings() {
        navigateTo("AccountSettings", null, true, false)
    }

    function navigateTo(page, params, animate, clearHistory) {

        const pageName = "%1Page".arg(page)
        const path = "./pages/%1.qml".arg(pageName)

        // cancel navigation if the page is already shown
        if (stackView.currentItem.toString().startsWith(pageName)){
            return
        }

        if (clearHistory) {
            stackView.clear()
        }

        if (animate) {
            stackView.push(path, params)
        }
        else {
            stackView.push(path, params, StackView.Immediate)
        }
    }
}
