import QtQuick 2.15
import QtQuick.Layouts 1.15

import "../theme"

ListDelegate {
    height: Theme.headerHeight
    openable: false

    Avatar {
        id: avatar
        nickname: model.title
    }

    TwoLineLabel {
        Layout.fillWidth: true
        title: model.title
        description: model.lastMessageBody
    }

    Column {
        width: 30
        spacing: Theme.minSpacing

        MessageCounter {
           count: model.unreadMessageCount
           anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: model.lastEventTime
            color: Theme.secondaryTextColor
            font.pointSize: UiHelper.fixFontSz(9)
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
