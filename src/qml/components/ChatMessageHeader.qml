import QtQuick 2.15

import "../components"

Column {
    id: root

    ChatDateSeporator {
        visible: model.firstInSection
        date: model.day
        height: 40
        width: root.width
    }

    UnreadMessagesSeparator {
        id: unreadMessagesSeparator
        visible: false
        width: parent.width
        height: 1
    }
}
