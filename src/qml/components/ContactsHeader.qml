import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

ToolBar {
    id: toolbarId
    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property alias showSeporator: seporator.visible
    // search
    property alias search: searchId.search
    property alias isSearchOpen: searchId.isSearchOpen
    property alias searchPlaceholder: searchId.searchPlaceholder

    default property alias menu: contextMenu.contentData

    background: Rectangle {
        implicitHeight: 60
        color: Theme.contactsBackgroundColor

        Rectangle {
            id: seporator
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
        anchors.leftMargin: 20
        anchors.rightMargin: 20

        Column {
            Layout.fillWidth: !isSearchOpen
            visible: !isSearchOpen

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
            Layout.fillWidth: isSearchOpen
            Layout.preferredWidth: 48
            height: 40

            Search {
                id: searchId
            }
        }

        ImageButton {
            visible: !isSearchOpen

            id: menuButton
            image: "More"
            // visible: menu.length
            opacity: menu.length ? 1 : 0
            enabled: menu.length
            onClicked: contextMenu.open()

            ContextMenu {
                id: contextMenu
                dropdown: true
            }
        }
    }
}
