import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Row {
    spacing: Theme.spacing

    Label {
        text: qsTr("Server")
        color: Theme.primaryTextColor
        horizontalAlignment: Text.AlignHCenter
    }

    Label {
        text: qsTr("Default")
        color: Theme.secondaryTextColor
        horizontalAlignment: Text.AlignHCenter
    }
}
