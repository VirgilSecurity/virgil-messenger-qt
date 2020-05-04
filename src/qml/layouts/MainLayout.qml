import QtQuick 2.12
import QtQuick.Controls 2.12

import "../js/login.js" as LoginLogic

StackView {
    focus: true
    anchors.fill: parent

    initialItem: Qt.createComponent("../pages/AuthenticationPage.qml")

    Keys.onReleased: {
        console.log('event in event', event)
        if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) back(event)
    }

    property var chatLayout

    function back(event) {
        console.log('event in handler', event)
        if (depth > 1) {
            event.accepted = true
            pop()
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
            if (!chatLayout) {
                chatLayout = Qt.createComponent("./ChatLayout.qml").createObject(parent)
            }
            Messenger.signIn(user)
            showPopupInform("Sign In ...")
            replace(chatLayout)
        } else {
            root.showPopupError(qsTr("Incorrect user name"))
        }
    }

    // Sign up
    function signUp(user) {
        if (LoginLogic.validateUser(user)) {
            if (!chatLayout) {
                chatLayout = Qt.createComponent("./ChatLayout.qml").createObject(parent)
            }
            Messenger.signUp(user)
            showPopupInform("Sign Up ...")
            replace(chatLayout)
        } else {
            showPopupError(qsTr("Incorrect user name"))
        }
    }

    function logout() {
        Messenger.logout()

        showAuthentication()
    }

    function showUserSettings() {
        chatLayout.showUserSettings()
    }
}
