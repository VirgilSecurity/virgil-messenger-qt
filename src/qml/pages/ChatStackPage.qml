import QtQuick 2.15
import QtQuick.Controls 2.15

import "../components"
import "../theme"

StackView {
    id: stackView

    QtObject {
        id: d
        readonly property var manager: app.stateManager

        function onCurrentStateChanged(state) {
            if (state === manager.chatState) {
                const states = [manager.chatListState, manager.newChatState, manager.newGroupChatState, manager.downloadKeyState]
                if (states.includes(manager.previousState)) {
                    stackView.push("ChatPage.qml", StackView.Transition)
                }
            }
        }

        function onGoBack() {
            if (manager.currentState === manager.chatState) {
                stackView.pop()
            }
        }
    }

    initialItem: "ChatListPage.qml"

    Connections {
        target: d.manager

        function onCurrentStateChanged(state) { d.onCurrentStateChanged(state) }
        function onGoBack() { d.onGoBack() }
    }
}
