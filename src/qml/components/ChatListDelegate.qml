import QtQuick 2.15
import QtQuick.Layouts 1.15

import "../theme"

ListDelegate {
    Avatar {
        id: avatar
        nickname: model.contactId
    }

    Column {
        Layout.fillWidth: true
        clip: true

        Text {
            color: Theme.primaryTextColor
            font.pointSize: UiHelper.fixFontSz(15)
            text: model.contactId
        }

        Text {
            id: messageBody
            color: Theme.secondaryTextColor
            font.pointSize: UiHelper.fixFontSz(12)
            width: parent.width
            text: model.lastMessageBody
            elide: Text.ElideRight
        }
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
