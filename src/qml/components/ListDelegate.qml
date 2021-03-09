import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import "../base"
import "../theme"

ItemDelegate {
    id: root

    property alias backgroundColor: backgroundItem.color
    property real leftMargin: Theme.smallSpacing
    property real rightMargin: Theme.smallSpacing

    property var selectionModel: undefined
    property bool openable: true
    default property alias children: rowLayout.children

    signal openItem()
    signal selectItem(bool multiSelect)

    background: Rectangle {
        id: backgroundItem
        color: (root.down || model.isSelected) ? Theme.contactPressedColor : "transparent"
        radius: 6
    }

    contentItem: RowLayout {
        id: rowLayout
        anchors {
            fill: parent
            leftMargin: root.leftMargin
            rightMargin: root.rightMargin
        }
        spacing: Theme.smallSpacing
    }

    onClicked: {
        if (Platform.isDesktop) {
            root.selectItem(app.keyboardModifiers() & Qt.ControlModifier)
        }
        else if (!root.openable || root.selectionModel && root.selectionModel.hasSelection) {
            root.selectItem(true)
        }
        else {
            root.openItem()
        }
    }

    onDoubleClicked: {
        if (root.openable && Platform.isDesktop) {
            root.openItem()
        }
    }

    onPressAndHold: {
        if (Platform.isMobile) {
            root.selectItem(true)
        }
    }
}
