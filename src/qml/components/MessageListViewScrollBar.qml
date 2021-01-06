import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

ScrollBar {
    id: root

    anchors {
        right: parent.right
        rightMargin: -Theme.margin
    }
}
