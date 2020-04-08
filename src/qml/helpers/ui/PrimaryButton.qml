import QtQuick 2.6
import QtQuick.Controls 2.0

import "../../theme"

Button {
    id: control
    font.pointSize: 20

    property alias name: control.text
    property color baseColor
    property color borderColor

    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: Theme.buttonPrimaryTextColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        id: bgrect
        implicitWidth: 100
        implicitHeight: 44
        color: Theme.buttonPrimaryColor
        opacity: control.down ? 0.7 : 1
        radius: height/2
    }
}
