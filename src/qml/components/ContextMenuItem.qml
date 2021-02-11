import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

MenuItem {
    id: menuItem
    implicitWidth: d.contextMenu.width
    implicitHeight: visible ? d.itemHeight : 0
    font.pointSize: UiHelper.fixFontSz(15)

    property string iconName: ""

    QtObject {
        id: d

        readonly property var contextMenu: menuItem.parent
        readonly property real itemHeight: contextMenu.compact ? 30 : 40
        readonly property real itemPadding: contextMenu.compact ? Theme.smallPadding : Theme.padding
        readonly property real spacing: 12
    }

    contentItem: Row {
        spacing: d.spacing
        leftPadding: d.itemPadding
        rightPadding: d.itemPadding

        Image {
            id: multiselectAvatarItem
            anchors.verticalCenter: parent.verticalCenter
            height: d.itemHeight
            source: iconName === "" ? "" : "../resources/icons/%1.png".arg(iconName)
            visible: iconName !== ""
            fillMode: Image.PreserveAspectFit
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: menuItem.text
            font: menuItem.font
            color: Theme.primaryTextColor
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    background: Rectangle {
        implicitWidth: d.contextMenu.implicitWidth
        implicitHeight: d.contextMenu.implicitHeight
        color: menuItem.highlighted ? "#59717D" : Theme.menuBackgroundColor
    }
}
