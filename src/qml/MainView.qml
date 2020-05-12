import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1

import "./theme"
import "./helpers/login.js" as LoginLogic

Control {
    id: mainView
    anchors.fill: parent

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    property string lastSignedInUser

    Settings {
        property alias lastSignedInUser: mainView.lastSignedInUser
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        StackView {
            id: stackView
            Layout.fillHeight: true
            Layout.fillWidth: true
            z: 1

            background: Rectangle {
                color: Theme.contactsBackgroundColor
            }
        }
    }

    Component.onCompleted: {        
        showSplashScreen()
    }

    function signIn(user) {
        if (LoginLogic.validateUser(user)) {
            Messenger.signIn(user)
            stackView.clear()
            lastSignedInUser = user
            showContacts()
        } else {
            root.showPopupError(qsTr("Incorrect user name"))
        }
    }

    function signUp(user) {
        if (LoginLogic.validateUser(user)) {
            Messenger.signUp(user)
            showPopupInform("Sign Up ...")
            lastSignedInUser = user
            showContacts()
        } else {
            showPopupError(qsTr("Incorrect user name"))
        }
    }

    function signOut() {
        Messenger.logout()

        // clear all pages in the stackview and push sign in page
        // as a first page in the stack
        stackView.clear()
        lastSignedInUser = ""
        showAuth(true)
    }


    function chatWith(recipient) {
        ConversationsModel.recipient = recipient
        stackView.push("./pages/ChatPage.qml")
    }

    // Navigation
    //
    // All the app navigation must be done throught executing
    // the functions below.

    function back() {
        stackView.pop()
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

    function showSignInAs() {
        stackView.push("./pages/SignInAsPage.qml")
    }

    function showRegister() {
        stackView.push("./pages/RegisterPage.qml")
    }

    function showContacts() {
        stackView.push("./pages/ContactsPage.qml")
    }

    function showAccountSettings() {
        stackView.push("./pages/AccountSettingsPage.qml")
    }
}
