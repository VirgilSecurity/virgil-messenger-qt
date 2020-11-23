import QtQuick 2.12
import QtQuick.Controls 2.12

Button {
    id: button

    property string image: "Grid"
    property string imageSource: ""
    property bool disabled: false
    property alias backgroundColor: background.color

    icon.color: "transparent"
    icon.source: "../resources/icons/%1.png".arg(image)

    padding: 0

    background: Rectangle {
        id: background
        color: "transparent"
        implicitWidth: 40
        implicitHeight: 40
        radius: 20
    }

    opacity: disabled ? 0.3 : 1

    Rectangle {
        id: hoverBackground
        anchors.centerIn: parent
        width: 40
        height: 40
        color: "white"
        opacity: 0.07
        radius: 20
        visible: button.hovered && !button.disabled
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
            PropertyChanges { target: hoverBackground; width: 38 }
            PropertyChanges { target: hoverBackground; height: 38 }
            PropertyChanges { target: button; icon.height: 23 }
            PropertyChanges { target: button; icon.width: 23 }
        },
        State {
            name: "RELEASED"
            PropertyChanges { target: hoverBackground; width: 40 }
            PropertyChanges { target: hoverBackground; height: 40 }
            PropertyChanges { target: button; icon.height: 24 }
            PropertyChanges { target: button; icon.width: 24 }
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
