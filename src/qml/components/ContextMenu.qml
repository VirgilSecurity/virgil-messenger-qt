import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"

Menu {
    id: root

    property bool dropdown: false
    property bool compact: false

    QtObject {
        id: d

        readonly property real spacing: 5
        readonly property real menuWidth: calculateWidth(compact ? 150 : 200)
        readonly property real vPadding: compact ? 5 : 10
        readonly property real hPadding: 0
        readonly property real itemHeight: compact ? 30 : 40
        readonly property real itemPadding: compact ? 10 : 20

        function updatePosition() {
            if (!dropdown || !root.visible)
                return;
            var window = mainView.parent
            var button = root.parent
            var coord = button.mapToItem(null, 0, 0)
            var centerX = coord.x + 0.5 * button.width
            var centerY = coord.y + 0.5 * button.height
            root.x = (centerX > 0.5 * window.width) ? (button.width - root.width) : 0
            root.y = (centerY > 0.5 * window.height) ? (-spacing - root.height) : (button.height + spacing)
        }

        Component.onCompleted: root.visibleChanged.connect(updatePosition)
    }

    implicitWidth: d.menuWidth
    topPadding: d.vPadding
    bottomPadding: d.vPadding
    leftPadding: d.hPadding
    rightPadding: d.hPadding

    background: Rectangle {
        color: Theme.menuBackgroundColor
        radius: 6
    }

    delegate: MenuItem {
        id: menuItem
        implicitWidth: parent.width
        implicitHeight: d.itemHeight
        font.pointSize: UiHelper.fixFontSz(15)

        contentItem: Text {
            leftPadding: d.itemPadding
            rightPadding: d.itemPadding
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

    function calculateWidth(maxWidth) {
        var width = 0;
        var padding = 0;
        for (var i = 0; i < count; ++i) {
            var item = itemAt(i);
            width = Math.max(item.contentItem.implicitWidth, width);
            padding = Math.max(item.padding, padding);
        }
        return Math.min(maxWidth, width + (padding + d.hPadding) * 2);
    }
}
