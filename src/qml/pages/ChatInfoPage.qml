import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"
import "../components"

Page {
    readonly property var appState: app.stateManager.chatState

    // FIXME(fpohtmeh): implement
    readonly property bool isGroup: true

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        title: isGroup ? qsTr("Group info") : qsTr("Chat info")
    }
}
