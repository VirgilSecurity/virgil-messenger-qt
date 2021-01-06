import QtQuick 2.15

import "../components"

Column {
    id: root

    Rectangle {
        color: "navy"
        height: 20
        width: root.width

        Text {
            text: qsTr("Replied from user1")
            color: "white"
        }
    }

    ChatDateSeporator {
        text: qsTr("today")
        height: 40
        width: root.width
    }
}
