import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

MenuItem {
    id: menuItem
    implicitWidth: d.contextMenu.width
    implicitHeight: visible ? d.itemHeight : 0
    font.pointSize: UiHelper.fixFontSz(15)

    QtObject {
        id: d

        readonly property var contextMenu: menuItem.parent
        readonly property real itemHeight: contextMenu.compact ? 30 : 40
        readonly property real itemPadding: contextMenu.compact ? Theme.smallPadding : Theme.padding
    }

    contentItem: Text {
        leftPadding: d.itemPadding
        rightPadding: d.itemPadding
        text: menuItem.text
        font: menuItem.font
        color: menuItem.enabled ? Theme.primaryTextColor : Qt.darker(Theme.primaryTextColor, 1.5)
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: d.contextMenu.implicitWidth
        implicitHeight: d.contextMenu.implicitHeight
        color: menuItem.highlighted ? "#59717D" : Theme.menuBackgroundColor
    }
}
