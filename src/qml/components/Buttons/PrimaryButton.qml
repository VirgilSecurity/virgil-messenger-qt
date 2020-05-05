import QtQuick 2.12
import QtQuick.Controls 2.12

import "../../theme"

Button {
    id: control
    font.pointSize: UiHelper.fixFontSz(15)

    property alias name: control.text
    property color baseColor
    property color borderColor

    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: Theme.primaryTextColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        id: bgrect
        implicitWidth: 100
        implicitHeight: 40
        color: Theme.buttonPrimaryColor
        opacity: control.down ? 0.7 : 1
        radius: height/2
    }
}
