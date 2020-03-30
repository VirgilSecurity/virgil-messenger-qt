import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "login/login.js" as LoginLogic
import "helpers/ui"

ApplicationWindow {
    id: rootWindow
    visible: true
    title: qsTr("Virgil IoTKit Qt Demo")
    minimumWidth: 550
    minimumHeight: 500

    //
    //  Properties
    //
    property color backGroundColor : "#394454"
    property color mainAppColor: "#6fda9c"
    property color mainTextCOlor: "#f0f0f0"

    property color toolbarTextColor: "white"
    property color toolbarColor: "#455462"
    property int   toolbarHeight: 40

    property bool mobileView: false

    // Mobile View Pages
    property var contactPage
    property var loginPage
    property var settingsPage

    //
    //  Connections
    //
    Connections {
        target: Messenger

        onFireError: {
            showPopupError(errorText)

            // Mobile
            mobileView.replace(loginPage)

            // Desktop
            desktopView.mode = desktopView.kModeLogin
        }

        onFireInform: {
            showPopupInform(informText)
        }

        onFireConnecting: {
            showPopupInform(qsTr("Connecting"))
        }

        onFireReady: {
            showPopupSucces(qsTr("Ready to chat"))
        }

        onFireAddedContact: {
        }

        onFireNewMessage: {
            showChat(from)
        }
    }

    // After loading show initial Login Page
    Component.onCompleted: {
        contactPage = Qt.createComponent("qrc:/qml/chat/ContactPage.qml")
        loginPage = Qt.createComponent("qrc:/qml/login/Login.qml")
        settingsPage = Qt.createComponent("qrc:/qml/settings/SettingsPage.qml")
        mobileView.replace(loginPage)
    }

    //
    //  UI
    //

    // Mobile view
    MobileView {
        id: mobileView
        visible: isMobileView()
    }

    // Desktop view
    DesktopView {
        id: desktopView
        visible: !isMobileView()
    }

    // Popup to show messages or warnings on the bottom postion of the screen
    Popup {
        id: inform
    }

    // Show Popup message
    function showPopup(message, color, textColor, isOnTop, isModal) {
        inform.popupColor = color
        inform.popupColorText = textColor
        inform.popupView.popMessage = message
        inform.popupOnTop = isOnTop
        inform.popupModal = isModal
        inform.popupView.open()
    }

    function showPopupError(message) {
        showPopup(message, "#b44", "#ffffff", true, true)
    }

    function showPopupInform(message) {
        showPopup(message, "#FFFACD", "#00", true, false)
    }

    function showPopupSucces(message) {
        showPopup(message, "#66CDAA", "#00", true, false)
    }

    // Show chat with
    function showChat(contact) {
        ConversationsModel.recipient = contact

        // Mobile
        mobileView.replace("qrc:/qml/chat/ConversationPage.qml", { inConversationWith: contact })

        // Desktop
        desktopView.chatView.inConversationWith = contact
    }

    // Show contacts
    function showContacts() {
        mobileView.replace(contactPage)
    }

    // Show settings
    function showSettings() {
        // Mobile
        mobileView.replace(settingsPage)

        // Desktop
        desktopView.mode = desktopView.kModeSettings
    }

    // Close settings
    function closeSettings() {
        // Mobile
        mobileView.replace(contactPage)

        // Desktop
        desktopView.mode = desktopView.kModeNormal
    }

    // Logout
    function logout() {
        Messenger.logout()

        // Mobile
        mobileView.replace(loginPage)

        // Desktop
        desktopView.mode = desktopView.kModeLogin
    }

    // View mode detection
    function isMobileView() {
        var _minSz = 640

        if (rootWindow.mobileView) {
            return true;
        }

        return rootWindow.width < _minSz;
    }

    // Sign in
    function signInUser(user) {
        if (LoginLogic.validateUser(user)) {
            Messenger.signIn(user)
            showPopupInform("Sign In ...")
            mobileView.replace(contactPage)
            desktopView.mode = desktopView.kModeNormal
        } else {
            showPopupError(qsTr("Incorrect user name"))
        }
    }

    // Sign up
    function signUpUser(user) {
        if (LoginLogic.validateUser(user)) {
            Messenger.signUp(user)
            showPopupInform("Sign Up ...")
            mobileView.replace(contactPage)
            desktopView.mode = desktopView.kModeNormal
        } else {
            showPopupError(qsTr("Incorrect user name"))
        }
    }
}
