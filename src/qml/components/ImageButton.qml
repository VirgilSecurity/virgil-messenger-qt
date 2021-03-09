import QtQuick 2.12
import QtQuick.Controls 2.12

Button {
    id: button

    property string image: "Grid"
    property string imageSource: ""
    property int imageSize: 40
    property int iconSize: 24
    property bool hoverVisible: true
    property alias backgroundColor: background.color

    icon.color: "transparent"
    icon.source: "../resources/icons/%1.png".arg(image)
    icon.width: iconSize
    icon.height: iconSize

    padding: 0

    background: Rectangle {
        id: background
        color: "transparent"
        implicitWidth: imageSize
        implicitHeight: imageSize
        radius: 0.5 * imageSize
    }

    opacity: enabled ? 1 : 0.3

    Rectangle {
        id: hoverBackground
        anchors.centerIn: parent
        width: imageSize
        height: imageSize
        color: "white"
        opacity: 0.07
        radius: 0.5 * imageSize
        visible: button.hoverVisible && button.hovered
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onPressed: {
            if (!button.disabled) {
                button.state = "PRESSED"
            }
        }
        onReleased: {
            if (!button.disabled) {
                button.clicked()
                button.state = "RELEASED"
            }
        }
    }

    states: [
        State {
            name: "PRESSED"
            PropertyChanges { target: hoverBackground; width: imageSize - 2 }
            PropertyChanges { target: hoverBackground; height: imageSize - 2 }
            PropertyChanges { target: button; icon.height: iconSize - 1 }
            PropertyChanges { target: button; icon.width: iconSize - 1 }
        },
        State {
            name: "RELEASED"
            PropertyChanges { target: hoverBackground; width: imageSize }
            PropertyChanges { target: hoverBackground; height: imageSize }
            PropertyChanges { target: button; icon.height: iconSize }
            PropertyChanges { target: button; icon.width: iconSize }
        }
    ]

    transitions: [
        Transition {
            from: "PRESSED"
            to: "RELEASED"
            PropertyAnimation { target: hoverBackground; properties: "width, height"; duration: 50}
            PropertyAnimation { target: button; properties: "icon.width, icon.height"; duration: 50}
        },
        Transition {
            from: "RELEASED"
            to: "PRESSED"
            PropertyAnimation { target: hoverBackground; properties: "width, height"; duration: 50}
            PropertyAnimation { target: button; properties: "icon.width, icon.height"; duration: 50}
        }
    ]
}
