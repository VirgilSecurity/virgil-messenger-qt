import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "login/login.js" as Backend
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

    //
    //  Connections
    //
    Connections {
        target: Messenger

        onFireError: {
            showPopupError(errorText)
            mobileView.push("qrc:/qml/login/Login.qml")
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

    // After loading show initial Login Page
    Component.onCompleted: {
        mobileView.push("qrc:/qml/login/Login.qml")
    }

    // Popup to show messages or warnings on the bottom postion of the screen
    Popup {
        id: inform
    }

    // Show Popup message
    function showPopupError(message) {
        inform.popupColor = "#b44"
        inform.popupColorText = "#ffffff"
        inform.popupView.popMessage = message
        inform.popupOnTop = true
        inform.popupModal = true
        inform.popupView.open()
    }

    function showPopupInform(message) {
        inform.popupColor = "#FFFACD"
        inform.popupColorText = "#00"
        inform.popupView.popMessage = message
        inform.popupOnTop = true
        inform.popupModal = false
        inform.popupView.open()
    }

    function showPopupSucces(message) {
        inform.popupColor = "#66CDAA"
        inform.popupColorText = "#00"
        inform.popupView.popMessage = message
        inform.popupOnTop = true
        inform.popupModal = false
        inform.popupView.open()
    }

    // Show chat with
    function showChat(contact) {
        ConversationsModel.recipient = contact

        // Mobile
        mobileView.push("qrc:/qml/chat/ConversationPage.qml", { inConversationWith: contact })

        // Desktop
        desktopView.chatView.inConversationWith = contact
    }

    // Show contacts
    function showContacts() {
        mobileView.push("qrc:/qml/chat/ContactPage.qml")
    }

    // View mode detection
    function isMobileView() {
        var _minSz = 640

        if (rootWindow.mobileView) {
            return true;
        }

        return rootWindow.width < _minSz;
    }
}
