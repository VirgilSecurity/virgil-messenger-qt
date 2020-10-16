import QtQuick 2.12
import QtQuick.Controls 2.12

import "../theme"

Item {
    id: containerId
    anchors.centerIn: parent
    height: parent.height

    property alias searchPlaceholder: searchField.placeholderText
    property alias search: searchField.text
    property bool isSearchOpen: state === "open"

    Behavior on width {
        NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
    }

    state: "closed"
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
                target: searchField
                text: ""
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
            name: "closed"

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
                text: ""
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
        activeFocusOnPress: true
        leftPadding: 38
        rightPadding: 30
        font.pixelSize: UiHelper.fixFontSz(15)
        placeholderTextColor: "#59717D"

        color: "white"

        background: Rectangle {
            id: backgroundId
            radius: 20
            color: "transparent"
        }

        Keys.onPressed: {
            if (containerId.state === "open" && (event.key === Qt.Key_Back || event.key === Qt.Key_Escape)) {
                containerId.state = "closed"
                event.accepted = true;
            }
        }
    }

    ImageButton {
        id: searchButtonId
        image: "Search"
        anchors.verticalCenter: parent.verticalCenter
        enabled: !isSearchOpen || !searchField.activeFocus

        onClicked: {
            containerId.state = "open"
            searchField.forceActiveFocus()
        }
    }

    ImageButton {
        id: closeButtonId
        image: "Close"
        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
        }

        onClicked: {
            containerId.state = "closed"
        }
    }
}
