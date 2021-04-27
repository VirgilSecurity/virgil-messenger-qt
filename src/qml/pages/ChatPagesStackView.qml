import QtQuick 2.15
import QtQuick.Controls 2.15

import "../base"

NavigationStackView {
    id: root

    signal newChatRequested()
    signal newGroupChatRequested()
    signal infoRequested()

    QtObject {
        id: d
        readonly property string chatId: controllers.chats.current.id
        readonly property bool isChatOpened: chatId.length > 0
        readonly property var stackView: Platform.isDesktop ? root : mainStackView

        onIsChatOpenedChanged: {
            if (isChatOpened) {
                stackView.navigatePush(chatPageComponent)
            } else {
                stackView.navigatePop()
            }
        }
    }

    Component {
        id: chatListComponent
        ChatListPage {
            onNewChatRequested: root.newChatRequested()
            onNewGroupChatRequested: root.newGroupChatRequested()
        }
    }

    Component {
        id: chatPageComponent
        ChatPage {
            onInfoRequested: root.infoRequested()
        }
    }

    Component.onCompleted: {
        navigateReplace(chatListComponent)
        if (d.isChatOpened) {
            d.stackView.navigatePush(chatPageComponent, {}, StackView.Immediate)
        }
    }

    function navigateBack(transition) { return d.isChatOpened && currentItem.navigateBack(transition) }
}
