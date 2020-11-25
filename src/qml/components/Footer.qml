import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"

Item {
    property string _text: ""
    height: 40
    width: parent.width

    Text {
        anchors.centerIn: parent
        text: _text
        font.letterSpacing: 0.23
        verticalAlignment: Text.AlignBottom
        color: Theme.labelColor
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: UiHelper.fixFontSz(10)
    }
}
