import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

Item {
    property alias popupView: popup

    Popup {
        id: popup

        property alias popMessage: message.text

        background: Rectangle {
            implicitWidth: rootWindow.width
            implicitHeight: 60
            color: popupBackGroundColor
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

        y: 0
        modal: true
        focus: true

        closePolicy: Popup.CloseOnPressOutside

        Text {
            id: message
            anchors.centerIn: parent
            color: popupTextCOlor
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

