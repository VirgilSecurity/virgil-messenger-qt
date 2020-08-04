import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "./components/Dialogs"
import "./components/Popups"
import "./components"
import "base"
import "theme"

ApplicationWindow {
    id: window
    visible: true
    title: qsTr("Virgil Secure Communications Platform") + (settings.devMode && messenger.user ? " - [%1]".arg(messenger.user) : "")
    minimumWidth: 320
    minimumHeight: 600

    Connections {
        target: messenger
        onCredentialsRequested: mainView.showAuth(signOut)
    }

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

    Popup {
        id: inform
    }

    SendCrashReportDialog {
        id: sendCrashReportDialog
    }

    Shortcut {
        sequence: StandardKey.Refresh
        enabled: settings.devMode
        onActivated: {
            window.close()
            app.reloadQml()
        }
    }

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

    Component.onCompleted: {
        Platform.detect()
        crashReporter.requested.connect(sendCrashReportDialog.open)
    }
}
