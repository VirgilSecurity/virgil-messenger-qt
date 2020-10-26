import QtQuick 2.12
import QtQuick.Controls 2.12

import "../theme"

Rectangle {
    id: containerId
    anchors.centerIn: parent
    height: parent.height
    color: Theme.inputBackgroundColor
    radius: 20

    Behavior on width {
        NumberAnimation {
            id: widthAnimation
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    property alias searchPlaceholder: searchField.placeholderText
    property alias search: searchField.text
    property bool isSearchOpen: state === "open"
    property alias isAnimationRunning: widthAnimation.running

    state: "closed"
    states: [
        State {
            name: "open"

            PropertyChanges {
                target: containerId
                width: parent.width
            }

            PropertyChanges {
                target: searchField
                text: ""
            }

            PropertyChanges {
                target: searchButtonId
                width: 24
                height: 24
                icon.color: "white"
                anchors {
                    left: parent.left
                    leftMargin: 10
                }
            }
        },
        State {
            name: "closed"

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
                    left: undefined
                    leftMargin: undefined
                }
            }
        }
    ]

    TextField {
        id: searchField

        anchors {
            fill: parent
            leftMargin: 30
            rightMargin: 30
        }
        activeFocusOnPress: true
        font.pixelSize: UiHelper.fixFontSz(15)
        placeholderTextColor: "#59717D"
        color: "white"

        background: Item {
        }

        Keys.onPressed: {
            if (isSearchOpen && (event.key === Qt.Key_Back || event.key === Qt.Key_Escape)) {
                containerId.state = "closed"
                event.accepted = true;
            }
        }
    }

    ImageButton {
        id: searchButtonId
        anchors.verticalCenter: parent.verticalCenter
        image: "Search"
        enabled: !isSearchOpen || !searchField.activeFocus
        visible: isSearchOpen || !isAnimationRunning

        onClicked: {
            containerId.state = "open"
            searchField.forceActiveFocus()
        }
    }

    ImageButton {
        id: closeButtonId
        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
        }
        image: "Close"

        onClicked: {
            containerId.state = "closed"
        }
    }
}
