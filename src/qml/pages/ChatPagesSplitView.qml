import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

SplitView {
    id: root

    signal newChatRequested()
    signal newGroupChatRequested()
    signal infoRequested()

    QtObject {
        id: d
        readonly property string chatId: controllers.chats.current.id
        readonly property bool isChatOpened: chatId.length > 0
    }

    handle: Rectangle {
        implicitWidth: Theme.landscapeMode.splitHandleSize
        implicitHeight: Theme.landscapeMode.splitHandleSize
        color: Theme.chatBackgroundColor
    }

    ChatListPage {
        id: chatListPage
        SplitView.fillHeight: true
        SplitView.fillWidth: false
        SplitView.preferredWidth: settings.chatListLandscapeWidth ? settings.chatListLandscapeWidth : Theme.landscapeMode.listMinimumWidth
        SplitView.minimumWidth: Theme.landscapeMode.listMinimumWidth
        SplitView.maximumWidth: Math.min(Theme.landscapeMode.listMaximumWidth,
                                         root.width - Theme.landscapeMode.chatMinimumWidth - Theme.landscapeMode.splitHandleSize)

        onNewChatRequested: root.newChatRequested()
        onNewGroupChatRequested: root.newGroupChatRequested()

        Component.onDestruction: settings.chatListLandscapeWidth = width
    }

    ChatPage {
        id: chatPage
        visible: d.isChatOpened
        showBackButton: false
        SplitView.minimumWidth: Theme.landscapeMode.chatMinimumWidth
        SplitView.fillHeight: true
        SplitView.fillWidth: true

        onInfoRequested: root.infoRequested()
    }

    Rectangle {
        visible: !d.isChatOpened
        color: Theme.chatBackgroundColor
        SplitView.minimumWidth: Theme.landscapeMode.chatMinimumWidth
        SplitView.fillHeight: true
        SplitView.fillWidth: true
    }

    function navigateBack(transition) { return d.isChatOpened && chatPage.navigateBack(transition) }
}
