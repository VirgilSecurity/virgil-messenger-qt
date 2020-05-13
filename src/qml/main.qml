import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtMultimedia 5.12
import QuickFuture 1.0

import "./components/Popups"
import "theme"

ApplicationWindow {
    id: root
    visible: true
    title: qsTr("Virgil Messenger")
    minimumWidth: 320
    minimumHeight: 600

    //
    //  Connections
    //
    Connections {
        target: Messenger

        onFireError: {
            showPopupError(errorText)
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

    // THE MainView of the Application!


    MainView {
        id: mainView
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

    // Shortcuts for hackers
    Shortcut {
        // TODO: remove on production or add conditional dev flag!
        sequence: StandardKey.Refresh
        onActivated: {
            var future = Messenger.logout()
            Future.onFinished(future, function(value) {
              console.log("Logouts result: ", Future.result(future))
            })

            root.close()
            app.reloadQml()
        }
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
