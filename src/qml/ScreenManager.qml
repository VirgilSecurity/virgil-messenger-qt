import QtQuick 2.7
import QtQuick.Controls 2.4

import "login/login.js" as LoginLogic

StackView {
    focus: true
    anchors.fill: parent

    initialItem: Qt.createComponent("./login/Authentication.qml")

    property var chatWorkspace

    function back() {
        pop()
    }

    function showAuthentication() {
        replace(Qt.createComponent("./login/Authentication.qml"))
    }

    function showLogin() {
        push(Qt.createComponent("./login/Login.qml"))
    }

    function showRegister() {
        push(Qt.createComponent("./login/Register.qml"))
    }

    function signIn(user) {
        if (LoginLogic.validateUser(user)) {
            if (!chatWorkspace) {
                chatWorkspace = Qt.createComponent("./ChatWorkspace.qml").createObject(parent)
            }
            Messenger.signIn(user)
            showPopupInform("Sign In ...")
            replace(chatWorkspace)
        } else {
            root.showPopupError(qsTr("Incorrect user name"))
        }
    }

    // Sign up
    function signUp(user) {
        if (LoginLogic.validateUser(user)) {
            if (!chatWorkspace) {
                chatWorkspace = Qt.createComponent("./ChatWorkspace.qml").createObject(parent)
            }
            Messenger.signUp(user)
            showPopupInform("Sign Up ...")
            replace(chatWorkspace)
        } else {
            showPopupError(qsTr("Incorrect user name"))
        }
    }

    function logout() {
        Messenger.logout()

        showAuthentication()
    }

    function showUserSettings() {
        chatWorkspace.showUserSettings()
    }
}
