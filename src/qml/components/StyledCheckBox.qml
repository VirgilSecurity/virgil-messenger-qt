import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

CheckBox {
    id: root
    text: qsTr("CheckBox")

    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: implicitWidth
        anchors.verticalCenter: parent.verticalCenter
        radius: 0.5 * height
        color: "transparent"
        border.color: Theme.buttonPrimaryColor
        border.width: 2

        Rectangle {
            width: 0.5 * parent.width
            height: width
            x: 0.5 * (parent.height - height)
            y: 0.5 * (parent.width - width)
            radius: 0.5 * width
            color: parent.border.color
            visible: root.checked
        }
    }

    contentItem: Text {
        text: root.text
        font: root.font
        opacity: enabled ? 1.0 : 0.3
        color: root.down ? Theme.secondaryTextColor : Theme.primaryTextColor
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.indicator.width
    }
}
