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

    readonly property int deafultBarHeight: 40
    readonly property int defaultMargin: 20
    readonly property int smallMargin: 10

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
            anchors.leftMargin: defaultMargin
            anchors.rightMargin: defaultMargin

            height: 1
            color: Theme.chatBackgroundColor
            anchors.bottom: parent.bottom
        }
    }

    state: "closed"
    states: [
        State {
            name: "opened"

            PropertyChanges {
                target: titleDescriptionColumn
                anchors.leftMargin: -deafultBarHeight
                opacity: 0

            }

            PropertyChanges {
                target: searchContainer
                width: toolbarContentContainer.width
                anchors.rightMargin: 0
            }

            PropertyChanges {
                target: menuButton
                anchors.rightMargin: -deafultBarHeight
                opacity: 0
            }

        },
        State {
            name: "closed"

            PropertyChanges {
                target: titleDescriptionColumn
                anchors.leftMargin: 0
                opacity: 1

            }

            PropertyChanges {
                target: searchContainer
                width: deafultBarHeight
                anchors.rightMargin: deafultBarHeight
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
                duration: Theme.defaultAnimationDuration
            }
        }
    ]

    Item {
        id: contentRow
        anchors {
            fill: parent
            leftMargin: defaultMargin
            rightMargin: smallMargin
        }

        height: deafultBarHeight

        ImageButton {
            id: backButton
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            image: "Arrow-Left"
            width: showBackButton ? deafultBarHeight : 0
            onClicked: app.stateManager.goBack()
        }

        Item {
            id: toolbarContentContainer
            anchors {
                left: backButton.right
                leftMargin: showBackButton ? smallMargin : 0
                right: parent.right
            }

            height: parent.height

            Column {
                id: titleDescriptionColumn
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
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
                    rightMargin: deafultBarHeight
                }

                height: deafultBarHeight
                width: deafultBarHeight

                Search {
                    id: searchId
                    anchors.fill: parent
                    visible: true
                    onStateChanged: {
                        toolbarId.state = searchId.state
                    }

                    onClosed: {
                        searchId.state = "closed"
                    }
                }
            }

            ImageButton {
                id: menuButton
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                }
                image: "More"
                opacity: (menu.length > 0) ? 1 : 0
                enabled: menu.length > 0

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
