import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import QtMultimedia 5.12
import QuickFuture 1.0

import "./components/Popups"
import "./components/Dialogs"
import "./components"
import "base"
import "theme"

ApplicationWindow {
    id: root
    visible: true
    title: app.applicationDisplayName

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

    //
    //  Connections
    //
    Connections {
        target: Messenger

        function onFireError() {
        }

        function onFireInform() {
        }

        function onFireWarning(text) {
            showPopupError(text);
        }

        function onFireConnecting() {
        }

        function onFireReady() {
        }

        function onFireAddedContact() {
        }

        function onFireNewMessage() {
        }
    }

    onClosing: {
        if (Platform.isAndroid) {
            close.accepted = false
            if (preview.visible) {
                preview.visible = false
            }
            else {
                mainView.back()
            }
        }
        else if (Platform.isDesktop) {
            settings.windowGeometry = Qt.rect(x, y, width, height)
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

    SendReportAsk {
        id: sendReportAsk
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

    function showPopupSuccess(message) {
        showPopup(message, "#66CDAA", "#00", true, false)
    }

    // View mode detection
    function isMobileView() {

        if (root.mobileView) {
            return true;
        }

        return root.width < root.maxMobileWidth;
    }

    function openPreview(filePath) {
        console.log("Open preview:", filePath)
        preview.filePath = filePath
        preview.visible = true
    }

    Item {
        id: preview
        anchors.fill: parent
        visible: false

        property alias filePath: previewImage.source

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.5
        }

        Image {
            id: previewImage
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            autoTransform: true
        }

        Label {
            anchors {
                top: parent.top
                right: parent.right
                topMargin: 15
                rightMargin: 15
            }
            text: qsTr("Close")
            color: "white"
            font.bold: true
        }

        MouseArea {
            acceptedButtons: Qt.AllButtons
            anchors.fill: parent
            onClicked: preview.visible = false
            onWheel: wheel.accepted = true
        }
    }

    Component.onCompleted: {
        Platform.detect()
        Messenger.informationRequested.connect(showPopupInform)
//        crashReporter.crashReportRequested.connect(sendReportAsk.open)
//        crashReporter.reportSent.connect(showPopupSuccess)
//        crashReporter.reportSentErr.connect(showPopupError)
    }

    onActiveFocusItemChanged: {
//        console.log(activeFocusItem, activeFocusItem ? activeFocusItem.objectName : undefined)
    }
}
