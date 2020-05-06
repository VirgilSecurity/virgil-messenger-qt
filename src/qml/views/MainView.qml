import QtQuick 2.12
import QtQuick.Controls 2.12
import Qt.labs.settings 1.1

import "../js/login.js" as LoginLogic

StackView {
    id: mainView
    focus: true
    anchors.fill: parent
    initialItem: Qt.createComponent("../pages/SplashScreenPage.qml")

    Keys.onReleased: {
        if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
            if (depth > 1) {
                event.accepted = true
                back()
            }
        }
    }

    property string lastSignedInUser

    Settings {
        property alias lastSignedInUser: mainView.lastSignedInUser
    }

    function back() {
      pop()
    }

    function showMain() {
        if (lastSignedInUser){
            signIn(lastSignedInUser)
        }
        else {
            showAuthentication()
        }
    }

    function showAuthentication() {
        replace(Qt.createComponent("../pages/AuthenticationPage.qml"))
    }

    function showLogin() {
        push(Qt.createComponent("./pages/LoginPage.qml"))
    }

    function showRegister() {
        push(Qt.createComponent("../pages/RegisterPage.qml"))
    }

    function signIn(user) {
        if (LoginLogic.validateUser(user)) {
            Messenger.signIn(user)
            showPopupInform("Sign In ...")
            replace(Qt.createComponent("./ChatView.qml"))
            lastSignedInUser = user
        } else {
            root.showPopupError(qsTr("Incorrect user name"))
        }
    }

    // Sign up
    function signUp(user) {
        if (LoginLogic.validateUser(user)) {
            Messenger.signUp(user)
            showPopupInform("Sign Up ...")
            replace(Qt.createComponent("./ChatView.qml"))
            lastSignedInUser = user
        } else {
            showPopupError(qsTr("Incorrect user name"))
        }
    }

    function logout() {
        Messenger.logout()

        lastSignedInUser = ""
        showAuthentication()
    }

    function showUserSettings() {
        chatLayout.showUserSettings()
    }
}
