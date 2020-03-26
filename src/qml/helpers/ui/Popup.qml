import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

Item {
    property alias popupView: popup
    property alias popupColor: popupBackground.color
    property alias popupColorText: message.color
    property alias popupModal: popup.modal
    property bool popupOnTop: false

    Popup {
        id: popup

        property alias popMessage: message.text

        background: Rectangle {
            id: popupBackground
            implicitWidth: rootWindow.width
            implicitHeight: 60
        }

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
        }

        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
        }

        onAboutToHide: {
            popupClose.stop()
        }

        y: popupOnTop ? 0 : (rootWindow.height - 60)
        modal: true
        focus: true

        closePolicy: Popup.CloseOnPressOutside

        Text {
            id: message
            anchors.centerIn: parent
        }
        onOpened: popupClose.start()
    }

    // Popup will be closed automatically in 2 seconds after its opened
    Timer {
        id: popupClose
        interval: 2000
        onTriggered: popup.close()
    }
}

