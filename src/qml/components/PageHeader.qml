import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"

Control {
    id: root
    width: parent.width
    height: Theme.headerHeight
    z: 1

    property alias title: headerTitle.title
    property alias description: headerTitle.description

    property alias titleHorizontalAlignment: headerTitle.horizontalAlignment
    property alias showSeparator: headerBackground.showSeparator
    property alias titleClickable: headerTitle.clickable

    property alias rightControl: rightControlLoader.sourceComponent
    property alias contextMenu: menuButton.contextMenu
    property alias contextMenuVisible: menuButton.visible

    signal titleClicked()

    background: HeaderBackground {
        id: headerBackground
        separatorColor: Theme.chatSeparatorColor
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.smallMargin
        anchors.rightMargin: Theme.smallMargin

        ImageButton {
            image: "Arrow-Left"
            onClicked: app.stateManager.goBack()
        }

        HeaderTitle {
            id: headerTitle
            Layout.fillWidth: true
            Layout.leftMargin: Theme.smallMargin

            onClicked: root.titleClicked()
        }

        ImageButton {
            id: menuButton
            image: "More"
            onClicked: contextMenu.open()
            visible: contextMenu.count > 0

            property var contextMenu: ContextMenu {
            }
        }

        Loader {
            id: rightControlLoader
        }
    }

    onContextMenuChanged: {
        contextMenu.parent = menuButton
        contextMenu.dropdown = true
    }
}
