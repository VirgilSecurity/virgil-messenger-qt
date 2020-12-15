import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Rectangle {
    id: root
    property alias textValidator: searchField.validator
    property alias searchPlaceholder: searchField.placeholderText
    property string search: searchField.text + searchField.preeditText
    property bool isSearchOpen: state === "opened"
    property bool closeable: true

    signal closed()
    signal accepted()

    radius: 20
    color: "transparent"

    state: "closed"
    states: [
        State {
            name: "opened"

            PropertyChanges {
                target: root
                color: Theme.inputBackgroundColor
            }

            PropertyChanges {
                target: searchField
                text: ""
            }

            PropertyChanges {
                target: searchButton
                icon.color: "white"
            }
        },
        State {
            name: "closed"

            PropertyChanges {
                target: root
                color: "transparent"
            }

            PropertyChanges {
                target: closeButton
                visible: false
            }

            PropertyChanges {
                target: searchField
                visible: false
                text: ""
            }

            PropertyChanges {
                target: searchButton
                icon.color: "transparent"
            }
        }
    ]

    transitions: [
        Transition {
            from: "opened"; to: "closed"
            PropertyAnimation {
                properties: "color"
                easing.type: Easing.InExpo
                duration: Theme.animationDuration
            }
        },
        Transition {
            from: "closed"; to: "opened"
            PropertyAnimation {
                properties: "color"
                easing.type: Easing.OutExpo
                duration: Theme.animationDuration
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
            if (root.closeable && isSearchOpen && (event.key === Qt.Key_Back || event.key === Qt.Key_Escape)) {
                root.state = "closed"
                event.accepted = true;
            }
            else if (isSearchOpen && (event.key === Qt.Key_Enter || event.key === Qt.Key_Return)) {
                accepted()
                event.accepted = true
            }
        }
    }

    ImageButton {
        id: searchButton
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: Theme.smallMargin
        width: 24
        height: 24
        image: "Search"
        enabled: !isSearchOpen || !searchField.activeFocus

        onClicked: {
            root.state = "opened"
            searchField.forceActiveFocus()
        }
    }

    ImageButton {
        id: closeButton
        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
        }
        image: "Close"

        onClicked: {
            if (root.closeable) {
                closed()
            }
            else {
                searchField.text = ""
            }

        }
    }
}
