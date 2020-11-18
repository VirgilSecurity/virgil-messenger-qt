import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

ToolBar {
    id: toolbarId
    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property alias showSeparator: separator.visible
    property alias showDescription: descriptionLabel.visible
    property alias menuImage: menuButton.image
    property var showBackButton: false
    // search
    property alias search: searchId.search
    property alias isSearchOpen: searchId.isSearchOpen
    property alias searchPlaceholder: searchId.searchPlaceholder
    property var filterSource: undefined

    default property alias menu: contextMenu.contentData

    onIsSearchOpenChanged: {
        if (filterSource) {
            filterSource.filter = ""
        }
        focus = false
        parent.focus = true
    }

    onSearchChanged: {
        if (filterSource) {
            filterSource.filter = search
        }
    }

    background: Rectangle {
        implicitHeight: 60
        color: Theme.contactsBackgroundColor

        Rectangle {
            id: separator
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 20
            anchors.rightMargin: 20

            height: 1
            color: Theme.chatBackgroundColor
            anchors.bottom: parent.bottom
        }
    }

    state: "closed"
    states: [
        State { // open
            name: "open"

            PropertyChanges {
                target: title_descr_Column
                anchors.leftMargin: -40
                opacity: 0

            }

            PropertyChanges {
                target: searchContainer
                width: toolbarContent_Container.width
                anchors.rightMargin: 0
            }

            PropertyChanges {
                target: menuButton
                anchors.rightMargin: -40
                opacity: 0
            }

        },
        State { // close
            name: "closed"

            PropertyChanges {
                target: title_descr_Column
                anchors.leftMargin: 0
                opacity: 1

            }

            PropertyChanges {
                target: searchContainer
                width: 40
                anchors.rightMargin: 40
            }

            PropertyChanges {
                target: menuButton
                anchors.rightMargin: 0
                opacity: 1
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                properties: "anchors.leftMargin, anchors.rightMargin, width, opacity"
                easing.type: Easing.InOutQuad
                duration: 250
            }
        }
    ]

    Item {
        id: contentRow
        anchors {
            fill: parent
            leftMargin: 20
            rightMargin: 10
        }

        height: 40

        ImageButton {
            id: backButton
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            image: "Arrow-Left"
            width: showBackButton ? 40 : 0
            onClicked: app.stateManager.goBack()
        }

        Item {
            id: toolbarContent_Container
            anchors {
                left: backButton.right
                leftMargin: showBackButton ? 10 : 0
                right: parent.right
            }

            height: parent.height

            Column { // Title and Description items
                id: title_descr_Column
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: 0
                }

                Label {
                    id: titleLabel
                    elide: Label.ElideRight

                    font.family: Theme.mainFont
                    font.pointSize: UiHelper.fixFontSz(15)
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter
                    font.bold: true
                    color: Theme.primaryTextColor
                }

                Label {
                    id: descriptionLabel
                    elide: Label.ElideRight

                    font.family: Theme.mainFont
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter
                    font.pointSize: UiHelper.fixFontSz(11)
                    color: Theme.secondaryTextColor
                }

            }

            Item {
                id: searchContainer
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                    rightMargin: 40
                }

                height: 40
                width: 40

                Search {
                    id: searchId
                    anchors.fill: parent
                    visible: true
                    onStateChanged: {
                        toolbarId.state = searchId.state
                    }
                }
            }

            ImageButton {
                id: menuButton
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                    rightMargin: 0
                }
                image: "More"
                opacity: menu.length ? 1 : 0
                enabled: {
                    if (!menu.length) {
                        return false
                    }
                    //                if (!isSearchOpen && !searchId.isAnimationRunning) {
                    //                    return true
                    //                } else {
                    //                    return false
                    //                }
                }

                onClicked: {
                    contextMenu.currentIndex = -1
                    contextMenu.open()
                }

                ContextMenu {
                    id: contextMenu
                    dropdown: true
                }
            }
        }
    }
}
