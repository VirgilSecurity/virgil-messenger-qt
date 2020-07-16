import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Item {
    property alias popupView: popup
    property alias popupColor: popupBackground.color
    property alias popupColorText: message.color
    property bool popupModal: true
    property bool popupOnTop: false

    Popup {
        id: popup

        property alias popMessage: message.text

        background: Rectangle {
            id: popupBackground
            implicitWidth: window.width
            implicitHeight: 25
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

        y: popupOnTop ? 0 : (window.height - 25)
        modal: popupModal
        focus: popupModal

        closePolicy: Popup.CloseOnPressOutside

        Text {
            id: message
            anchors.centerIn: parent
            font.pointSize: UiHelper.fixFontSz(12)
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

