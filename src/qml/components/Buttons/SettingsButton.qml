import QtQuick 2.12
import QtQuick.Controls 2.12

import "../../theme"

ToolButton {
    id: control
    icon.source: "../../resources/icons/More.png"
    icon.color: "transparent"
    width: 60
    height: parent.height

    property real menuWidth: 200
    property real menuItemHeight: 40
    property real menuItemPadding: 20
    // move components from direct childrens to menu childrens
    default property alias contentData: menu.contentData

    background: Rectangle {
        color: "transparent"
    }

    onClicked: menu.open()

		// TODO: Extract menu to separate component
    Menu {
        id: menu

        // coordinates relative to parent (ToolButton)
        x: control.width - menu.width - 5
        y: control.height + control.y

        implicitWidth: menuWidth

        topPadding: 10
        bottomPadding: 10

        background: Rectangle {
            color: Theme.menuBackgroundColor
            radius: 6
        }

        delegate: MenuItem {
            id: menuItem
            implicitWidth: parent.width
            implicitHeight: menuItemHeight
            font.pointSize: UiHelper.fixFontSz(15)

            contentItem: Text {
                leftPadding: menuItemPadding
                rightPadding: menuItemPadding
                text: menuItem.text
                font: menuItem.font
                color: menuItem.highlighted ? Theme.secondaryTextColor : Theme.primaryTextColor
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
    }
}
