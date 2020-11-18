import QtQuick 2.12
import QtQuick.Controls 2.12

import "../theme"

Rectangle {
    id: containerId

    radius: 20
    color: 'transparent'

    property alias searchPlaceholder: searchField.placeholderText
    property alias search: searchField.text
    property bool isSearchOpen: state === "open"
//    property alias isAnimationRunning: widthAnimation.running

    state: "closed"
    states: [
        State { // open
            name: "open"

            PropertyChanges {
                target: containerId
                color: Theme.inputBackgroundColor
            }

            PropertyChanges {
                target: searchField
                text: ""
            }

            PropertyChanges {
                target: searchButtonId
                icon.color: "white"
            }
        },
        State { // close
            name: "closed"

            PropertyChanges {
                target: containerId
                color: 'transparent'
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
                icon.color: "transparent"
            }
        }
    ]

    transitions: [
        Transition {
            from: 'open'; to: 'closed'
            PropertyAnimation {
                properties: "color"
                easing.type: Easing.InExpo
                duration: 250
            }
        },
        Transition {
            from: 'closed'; to: 'open'
            PropertyAnimation {
                properties: "color"
                easing.type: Easing.OutExpo
                duration: 250
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
        anchors.left: parent.left
        anchors.leftMargin: 10
        width: 24
        height: 24
        image: "Search"
        enabled: !isSearchOpen || !searchField.activeFocus

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
