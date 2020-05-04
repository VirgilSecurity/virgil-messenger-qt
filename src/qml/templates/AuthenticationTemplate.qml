import QtQuick 2.12
import QtQuick.Layouts 1.12

import "../theme"

ColumnLayout {
    id: layout
    anchors.fill: parent

    property Item content

    Item {
        id: topItem
        Layout.maximumHeight: (parent.height - content.height) / 2
        Layout.minimumHeight: 0
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    Text {
        id: bottomItem
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.maximumHeight: (parent.height - content.height) / 2
        Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
        Layout.topMargin: 15
        Layout.bottomMargin: 15
        text: qsTr("Powered by Virgil Security")
        verticalAlignment: Text.AlignBottom
        color: Theme.labelColor
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: UiHelper.fixFontSz(11)
    }

    onContentChanged: {
        data = [topItem, content, bottomItem]
    }
}
