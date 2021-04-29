import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import "../theme"

Menu {
    id: contextMenu
    x: d.menuPosition.x
    y: d.menuPosition.y

    property bool dropdown: false
    property bool compact: false

    QtObject {
        id: d

        readonly property real spacing: 5
        readonly property var menuPosition: calculatePosition()
        readonly property real menuWidth: d.calculateWidth(compact ? 150 : 250)
        readonly property real vPadding: compact ? 0.5 * Theme.smallPadding : Theme.smallPadding
        readonly property real hPadding: 0

        function calculatePosition() {
            if (!dropdown || !contextMenu.visible) {
                return Qt.point(0, 0);
            }
            var button = contextMenu.parent
            var coord = button.mapToItem(null, 0, 0)
            var centerX = coord.x + 0.5 * button.width
            var centerY = coord.y + 0.5 * button.height
            return Qt.point(
                (centerX > 0.5 * window.width) ? (button.width - contextMenu.width) : 0,
                (centerY > 0.5 * window.height) ? (-spacing - contextMenu.height) : (button.height + spacing)
            );
        }

        function calculateWidth(maxWidth) {
            var width = 0;
            var padding = 0;
            for (var i = 0; i < contextMenu.count; ++i) {
                const item = contextMenu.itemAt(i);
                width = Math.max(item.contentItem.implicitWidth, width);
                padding = Math.max(item.padding, padding);
            }
            return Math.min(maxWidth, width + (padding + hPadding) * 2);
        }
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

    delegate: ContextMenuItem {
    }

    onOpened: currentIndex = -1
}
