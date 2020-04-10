import QtQuick 2.6
import QtQuick.Controls 2.0
import "../../theme"

Button {
    id: control
    font.pointSize: 20

    property alias name: control.text

    contentItem: Text {
        text: control.text
        font: control.font
        color: Theme.primaryTextColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        id: bgrect
        implicitWidth: 100
        implicitHeight: 44
        color: "transparent"
        radius: height / 2
        border.color: Theme.buttonPrimaryColor
        border.width: 2
    }
}
