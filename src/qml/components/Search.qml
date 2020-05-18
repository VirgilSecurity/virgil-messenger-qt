import QtQuick 2.12
import QtQuick.Controls 2.12

import "../theme"

Item {
    id: containerId
    height: parent.height

    anchors.centerIn: parent

    Behavior on width {
        NumberAnimation { duration: 100 }
    }

    state: 'closed'
    states: [
        State {
            name: "open"

            ParentChange {
                target: searchButtonId
                parent: searchField
            }

            PropertyChanges {
                target: containerId
                width: parent.width
            }

            PropertyChanges {
                target: backgroundId
                color: Theme.inputBackgroundColor
            }

            PropertyChanges {
                target: searchButtonId
                width: 20
                height: 20
                icon.color: "white"
                anchors {
                    left: parent.left
                    leftMargin: 11
                    horizontalCenter: undefined
                }
            }
        },
        State {
            name: 'closed'

            ParentChange {
                target: searchButtonId
                parent: containerId
            }

            PropertyChanges {
                target: containerId
                width: 48
            }

            PropertyChanges {
                target: closeButtonId
                visible: false
            }

            PropertyChanges {
                target: searchField
                visible: false
                text: ''
            }

            PropertyChanges {
                target: searchButtonId
                width: 24
                height: 24
                icon.color: "transparent"
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    // clear previous anchor
                    left: undefined
                    leftMargin: undefined
                }
            }
        }
    ]

    TextField {
        id: searchField

        anchors.fill: parent;

        leftPadding: 38
        rightPadding: 30

        text: ''

        color: "white"

        background: Rectangle {
            id: backgroundId
            radius: 20
            color: "transparent"
        }

        ImageButton {
            id: closeButtonId
            imageSource: "../resources/icons/Close.png"
            anchors {
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: 6
            }
            onClicked: {
                containerId.state = 'closed'
            }
        }
    }

    ImageButton {
        id: searchButtonId
        anchors.verticalCenter: parent.verticalCenter

        imageSource: "../resources/icons/Search.png"

        onClicked: {
            containerId.state = "open"
        }
    }
}
