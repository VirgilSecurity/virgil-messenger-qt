import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"

RowLayout {
    Label {
        text: qsTr("Server")
        color: Theme.primaryTextColor
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
    }

    Label {
        text: qsTr("Default")
        color: Theme.secondaryTextColor
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
    }
}
