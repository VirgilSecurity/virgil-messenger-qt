import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12

import "../theme"

Menu {
    id: root

    property bool dropdown: false

    readonly property real menuWidth: 200
    readonly property real menuPadding: 10
    readonly property real menuItemHeight: 40
    readonly property real menuItemPadding: 20

    QtObject {
        readonly property int spacing: 5

        Component.onCompleted: root.visibleChanged.connect(function() {
            if (!dropdown || !root.visible)
                return;
            var button = root.parent
            var coord = button.mapToItem(null, 0, 0)
            var centerX = coord.x + 0.5 * button.width
            var centerY = coord.y + 0.5 * button.height
            root.x = (centerX > 0.5 * window.width) ? (button.width - root.width) : 0
            root.y = (centerY > 0.5 * window.height) ? (-spacing - root.height) : (button.height + spacing)
        })
    }

    implicitWidth: menuWidth
    topPadding: menuPadding
    bottomPadding: menuPadding

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
            color: Theme.primaryTextColor
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            implicitWidth: parent.implicitWidth
            implicitHeight: parent.implicitHeight
            color: menuItem.highlighted ? "#59717D" : root.background.color
        }
    }
}
