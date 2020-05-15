import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"

Menu {
    id: menu

    readonly property real menuWidth: 200
    readonly property real menuItemHeight: 40
    readonly property real menuItemPadding: 20

    // coordinates relative to parent (ToolButton)
    x: parent.width - menu.width - 5
    y: parent.height + parent.y

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
