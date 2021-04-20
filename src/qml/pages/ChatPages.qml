import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

Item {
    id: root

    signal newChatRequested()
    signal newGroupChatRequested()
    signal infoRequested()

    QtObject {
        id: d

        readonly property var lm: Theme.landscapeMode
        readonly property bool isLandscapeMode: Platform.isDesktop && width >= lm.listMinimumWidth + lm.splitHandleSize + lm.chatMinimumWidth
    }

    Component {
        id: splitViewComponent
        ChatPagesSplitView {
            anchors.fill: parent

            onNewChatRequested: root.newChatRequested()
            onNewGroupChatRequested: root.newGroupChatRequested()
            onInfoRequested: root.infoRequested()
        }
    }

    Component {
        id: stackViewComponent
        ChatPagesStackView {
            anchors.fill: parent

            onNewChatRequested: root.newChatRequested()
            onNewGroupChatRequested: root.newGroupChatRequested()
            onInfoRequested: root.infoRequested()
        }
    }

    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: d.isLandscapeMode ? splitViewComponent : stackViewComponent

        onItemChanged: window.updateAppState()
    }

    function navigateBack(transition) {
        if (controllers.chats.current.id.length > 0) {
            controllers.chats.closeChat()
            return true
        }
        return false
    }

    function currentPage() { return loader.item.currentPage() }
}
