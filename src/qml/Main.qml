import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtMultimedia 5.12

import "../qml/layouts"
import "./components/Popups"
import "theme"

ApplicationWindow {
    id: root
    visible: true
    title: qsTr("Virgil Messenger")
    minimumWidth: 320
    minimumHeight: 500

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    //
    //  Properties
    //
    property color backGroundColor : "#394454"
    property color mainAppColor: "#6fda9c"
    property color mainTextCOlor: "#f0f0f0"

    property color toolbarTextColor: "white"
    property color toolbarColor: "#455462"
    property int   toolbarHeight: 40

    property real maxMobileWidth: 640
    property bool mobileView: false

    //
    //  Connections
    //
    Connections {
        target: Messenger

        onFireError: {
            showPopupError(errorText)

            mainLayout.showAuthentication()
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
            messageSound.play()
            showChat(from)
        }
    }

    Shortcut {
        sequence: StandardKey.Refresh
        onActivated: {
            Messenger.logout()
            root.close()
            app.reloadQml()
        }
    }

    //
    //  UI
    //

    MainLayout {
        id: mainLayout
    }

    // Popup to show messages or warnings on the bottom postion of the screen
    Popup {
        id: inform
    }

    // Sound effect
    SoundEffect {
        id: messageSound
        source: "resources/sounds/message.wav"
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


    // View mode detection
    function isMobileView() {

        if (root.mobileView) {
            return true;
        }

        return root.width < root.maxMobileWidth;
    }

}
