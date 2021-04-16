import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

SplitView {
    id: root

    property bool isChatOpened: false
    readonly property var lm: Theme.landscapeMode

    signal newChatRequested()
    signal newGroupChatRequested()
    signal infoRequested()

    handle: Rectangle {
        implicitWidth: lm.splitHandleSize
        implicitHeight: lm.splitHandleSize
        color: Theme.chatBackgroundColor
    }

    ChatListPage {
        id: chatListPage
        SplitView.fillHeight: true
        SplitView.fillWidth: false
        SplitView.preferredWidth: settings.chatListLandscapeWidth ? settings.chatListLandscapeWidth : lm.listMinimumWidth
        SplitView.minimumWidth: lm.listMinimumWidth
        SplitView.maximumWidth: Math.min(lm.listMaximumWidth, root.width - lm.chatMinimumWidth - lm.splitHandleSize)

        onNewChatRequested: root.newChatRequested()
        onNewGroupChatRequested: root.newGroupChatRequested()

        Component.onDestruction: settings.chatListLandscapeWidth = width
    }

    ChatPage {
        id: chatPage
        visible: isChatOpened
        showBackButton: false
        SplitView.minimumWidth: lm.chatMinimumWidth
        SplitView.fillHeight: true
        SplitView.fillWidth: true

        onInfoRequested: root.infoRequested()
    }

    Rectangle {
        visible: !isChatOpened
        color: Theme.chatBackgroundColor
        SplitView.minimumWidth: lm.chatMinimumWidth
        SplitView.fillHeight: true
        SplitView.fillWidth: true
    }

    function enterState() {
        const appState = (isChatOpened ? chatPage : chatListPage).appState
        app.stateManager.enterState(appState)
    }
}
