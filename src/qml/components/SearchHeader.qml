import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"
import "../components"

// TODO(fpohtmeh): use PageHeader
ToolBar {
    id: toolbarId
    property alias title: headerTitle.title
    property alias description: headerTitle.description
    property alias showSeparator: headerBackground.showSeparator
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

    background: HeaderBackground {
        id: headerBackground
    }

    state: "closed"
    states: [
        State {
            name: "opened"

            PropertyChanges {
                target: headerTitle
                anchors.leftMargin: -searchId.recommendedHeight
                opacity: 0

            }

            PropertyChanges {
                target: searchContainer
                width: toolbarContentContainer.width
                anchors.rightMargin: 0
            }

            PropertyChanges {
                target: menuButton
                anchors.rightMargin: -searchId.recommendedHeight
                opacity: 0
            }

        },
        State {
            name: "closed"

            PropertyChanges {
                target: headerTitle
                anchors.leftMargin: 0
                opacity: 1

            }

            PropertyChanges {
                target: searchContainer
                width: searchId.recommendedHeight
                anchors.rightMargin: searchId.recommendedHeight
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
                duration: Theme.animationDuration
            }
        }
    ]

    Item {
        id: contentRow
        anchors {
            fill: parent
            leftMargin: Theme.margin
            rightMargin: Theme.smallMargin
        }
        height: searchId.recommendedHeight

        ImageButton {
            id: backButton
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            image: "Arrow-Left"
            width: showBackButton ? searchId.recommendedHeight : 0
            onClicked: app.stateManager.goBack()
        }

        Item {
            id: toolbarContentContainer
            anchors {
                left: backButton.right
                leftMargin: showBackButton ? Theme.smallMargin : 0
                right: parent.right
            }
            height: parent.height

            HeaderTitle {
                id: headerTitle
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                id: searchContainer
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                    rightMargin: searchId.recommendedHeight
                }

                height: searchId.recommendedHeight
                width: searchId.recommendedHeight

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
