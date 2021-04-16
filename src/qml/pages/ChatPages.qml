import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

Item {
    id: root

    readonly property bool isChatOpened: !controllers.chats.current.isNull

    signal newChatRequested()
    signal newGroupChatRequested()
    signal infoRequested()

    QtObject {
        id: d

        readonly property var lm: Theme.landscapeMode
        readonly property bool isLandscapeMode: Platform.isDesktop && width >= lm.listMinimumWidth + lm.splitHandleSize + lm.chatMinimumWidth
    }

    Component {
        id: chatSplitView
        ChatSplitView {
            anchors.fill: parent
            isChatOpened: root.isChatOpened

            onNewChatRequested: root.newChatRequested
            onNewGroupChatRequested: root.newGroupChatRequested
            onInfoRequested: root.infoRequested
        }
    }

    Component {
        id: chatStackView
        ChatStackView {
            anchors.fill: parent
            isChatOpened: root.isChatOpened

            onNewChatRequested: root.newChatRequested
            onNewGroupChatRequested: root.newGroupChatRequested
            onInfoRequested: root.infoRequested
        }
    }

    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: d.isLandscapeMode ? chatSplitView : chatStackView

        onItemChanged: root.enterState()
    }

    function navigateBack(transition) {
        if (isChatOpened) {
            controllers.chats.closeChat()
            return true
        }
        return false
    }

    function enterState() { loader.item.enterState() }
}
