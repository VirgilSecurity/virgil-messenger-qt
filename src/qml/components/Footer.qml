import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"

Item {
    height: 40
    width: parent.width

    Text {
        anchors.centerIn: parent
        text: qsTr("Powered by Virgil Security, Inc.")
        font.letterSpacing: 0.23
        verticalAlignment: Text.AlignBottom
        color: Theme.labelColor
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: UiHelper.fixFontSz(10)
    }
}
