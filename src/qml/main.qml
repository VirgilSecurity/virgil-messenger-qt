import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtMultimedia 5.12
import QuickFuture 1.0

import "./components/Popups"
import "./components"
import "base"
import "theme"

ApplicationWindow {
    id: window
    visible: true
    title: qsTr("Virgil Secure Communications Platform")
    minimumWidth: 320
    minimumHeight: 600

    property bool connectionError: false

    // TODO(fpohtmeh): remove
    /*
    //
    //  Connections
    //
    Connections {
        target: Messenger

        onFireError: {
        }

        onFireInform: {
        }

        onFireConnecting: {
        }

        onFireReady: {
        }

        onFireAddedContact: {
        }

        onFireNewMessage: {            
        }
    }
    */

    onClosing: {
        if (Platform.isAndroid) {
            close.accepted = false
            mainView.back()
        }
    }

    MainView {
        id: mainView
        anchors.fill: parent
    }

    // Popup to show messages or warnings on the bottom postion of the screen
    Popup {
        id: inform
    }

    // TODO(fpohtmeh): uncomment
    /*
    // Shortcuts for hackers
    Shortcut {
        // TODO: remove on production or add conditional dev flag!
        sequence: StandardKey.Refresh
        onActivated: {
            var future = Messenger.logout()
            Future.onFinished(future, function(value) {
              console.log("Logouts result: ", Future.result(future))
            })

            window.close()
            app.reloadQml()
        }
    }
    */

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

    function showPopupSuccess(message) {
        showPopup(message, "#66CDAA", "#00", true, false)
    }

    // View mode detection
    function isMobileView() {

        if (window.mobileView) {
            return true;
        }

        return window.width < window.maxMobileWidth;
    }

    Component.onCompleted: Platform.detect()
}
