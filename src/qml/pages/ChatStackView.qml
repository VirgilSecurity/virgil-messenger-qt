import QtQuick 2.15
import QtQuick.Controls 2.15

NavigationStackView {
    id: root

    property bool isChatOpened: false

    signal newChatRequested()
    signal newGroupChatRequested()
    signal infoRequested()

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
        if (isChatOpened) {
            navigatePush(chatPageComponent, {}, StackView.Immediate)
        }
    }

    onIsChatOpenedChanged: {
        if (isChatOpened) {
            navigatePush(chatPageComponent)
        } else {
            navigateBack()
        }
    }

    function enterState() {
        const appState = currentItem.appState
        app.stateManager.enterState(appState)
    }
}
