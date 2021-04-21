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

    property alias window: root

    visible: true
    title: (!settings.devMode || !controllers.users.currentUsername) ? app.applicationDisplayName : controllers.users.currentUsername

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
            close.accepted = !window.navigateBack()
        }
        else if (Platform.isDesktop) {
            settings.windowGeometry = Qt.rect(x, y, width, height)
        }
    }

    Shortcut {
        sequence: "Esc"
        onActivated: window.navigateBack()
        enabled: settings.devMode
    }

    MainView {
        id: mainView
        anchors.fill: parent
    }

    Item {
        anchors.fill: parent

        AttachmentPreview {
            id: attachmentPreview
            anchors.fill: parent
        }

        NetworkStatusControl {
            anchors {
                bottom: parent.bottom
                left: parent.left
                bottomMargin: Theme.smallMargin
                leftMargin: Theme.smallMargin
            }
        }

        AttachmentPicker { id: attachmentPicker }
        NotificationPopup { id: notificationPopup }
        KeyboardHandler { id: keyboardHandler }
        SendReportDialog { id: sendReportDialog }
    }

    function navigateBack(transition) {
        return attachmentPreview.navigateBack(transition) || mainView.navigateBack(transition)
    }

    Component.onCompleted: {
        app.notificationCreated.connect(function(text, error) {
            if (error) {
                notificationPopup.showError(text)
            } else {
                notificationPopup.showInform(text)
            }
        })
        crashReporter.crashReportRequested.connect(sendReportDialog.open)
        crashReporter.reportSent.connect(notificationPopup.showSuccess)
        crashReporter.reportErrorOccurred.connect(notificationPopup.showError)

        Platform.detect()
    }
}
