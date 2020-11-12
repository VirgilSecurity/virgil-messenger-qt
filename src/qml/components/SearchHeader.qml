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

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: loader.visible ? 8 : 20
        anchors.rightMargin: 20
        spacing: 10

        Loader {
            id: loader
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
            visible: showBackButton && !isSearchOpen && !searchId.isAnimationRunning

            ImageButton {
                anchors.fill: parent
                image: "Arrow-Left"
                onClicked: app.stateManager.goBack()
            }
        }

        Column {
            Layout.fillWidth: !isSearchOpen
            visible: !isSearchOpen && !searchId.isAnimationRunning
            clip: true

            Label {
                id: titleLabel
                elide: Label.ElideRight
                width: parent.width

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
                width: parent.width

                font.family: Theme.mainFont
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                font.pointSize: UiHelper.fixFontSz(11)
                color: Theme.secondaryTextColor
            }
        }

        Item {
            Layout.fillWidth: isSearchOpen || searchId.isAnimationRunning
            Layout.preferredWidth: 48
            height: 40

            Search {
                id: searchId
            }
        }

        ImageButton {
            visible: !isSearchOpen && !searchId.isAnimationRunning

            id: menuButton
            image: "More"
            opacity: menu.length ? 1 : 0
            enabled: menu.length
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
