import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import QtMultimedia 5.12

import "./components/Popups"
import "./components/Dialogs"
import "./components"
import "base"
import "theme"

ApplicationWindow {
    id: root
    visible: true
    title: (!settings.devMode || !controllers.users.userId) ? app.applicationDisplayName : controllers.users.userId

    Binding on x {
        when: Platform.isDesktop;
        value: d.restore ? settings.windowGeometry.x : 0.5 * (Screen.width - d.width)
    }
    Binding on y {
        when: Platform.isDesktop;
        value: d.restore ? settings.windowGeometry.y : 0.5 * (Screen.height - d.height)
    }
    Binding on width {
        when: Platform.isDesktop;
        value: d.restore ? settings.windowGeometry.width : d.width
    }
    Binding on height {
        when: Platform.isDesktop;
        value: d.restore ? settings.windowGeometry.height : d.height
    }
    Binding on minimumHeight { when: Platform.isDesktop; value: 500 }
    Binding on minimumWidth { when: Platform.isDesktop; value: 300 }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    QtObject {
        id: d
        readonly property int height: 800
        readonly property int width: 600
        readonly property bool restore: settings.windowGeometry.width > 0 && settings.windowGeometry.height > 0
    }

    onClosing: {
        if (Platform.isAndroid) {
            close.accepted = false
            app.stateManager.goBack()
        }
        else if (Platform.isDesktop) {
            settings.windowGeometry = Qt.rect(x, y, width, height)
        }
    }

    MainView {
        id: mainView
        anchors.fill: parent
        attachmentPreview: attachmentPreview
    }

    Item {
        anchors.fill: parent

        Popup {
            id: inform
        }

        AttachmentPreview {
            id: attachmentPreview
            anchors.fill: parent
            visible: false
        }

        SendReportDialog {
            id: sendReportDialog
        }

        AttachmentPicker {
            id: attachmentPicker
        }
    }


    // Show Popup message
    function showPopup(message, borderColor, popupBackgroundColor, textColor, interval) {
        inform.popupBorderColor = borderColor
        inform.popupBackgroundColor = popupBackgroundColor
        inform.popupColorText = textColor
        inform.popupText = message
        inform.popupInterval = interval
        inform.open()
    }

    function showPopupError(message, interval = 3000) {
        showPopup(message, "#b44", "black", "#ffffff", interval)
    }

    function showPopupInform(message, interval = 3000) {
        showPopup(message, "#FFFACD", "black", "#ffffff", interval)
    }

    function showPopupSuccess(message, interval = 3000) {
        showPopup(message, "#66CDAA", "black", "#ffffff", interval)
    }

    Timer {
        running: true
        repeat: false
        interval: 5000
        onTriggered: {
            showPopupError("This is new error popup!!!!!")
        }
    }

    Timer {
        running: true
        repeat: false
        interval: 10000
        onTriggered: {
            showPopupInform("This is only inform message. But now you can see how it work with long text. So, I think this implementation is better")
        }
    }

    Timer {
        running: true
        repeat: false
        interval: 17000
        onTriggered: {
            showPopupSuccess("This is success message!!!!")
        }
    }

    Component.onCompleted: {
        app.notificationCreated.connect(function(text, error) {
            if (error) {
                showPopupError(text)
            }
            else {
                showPopupInform(text)
            }
        })
        crashReporter.crashReportRequested.connect(sendReportDialog.open)
        crashReporter.reportSent.connect(showPopupSuccess)
        crashReporter.reportErrorOccurred.connect(showPopupError)

        Platform.detect()
        app.stateManager.setUiState()
    }

    onActiveFocusItemChanged: {
//        console.log(activeFocusItem, activeFocusItem ? activeFocusItem.objectName : undefined)
    }
}
