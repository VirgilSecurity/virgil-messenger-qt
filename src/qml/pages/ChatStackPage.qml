import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

Item {
    QtObject {
        id: d
        readonly property var manager: app.stateManager
        readonly property bool isLandscapeMode: Platform.isDesktop && width >= 540
        readonly property bool isChatOpened: !controllers.chats.current.isNull

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

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: "ChatListPage.qml"
        visible: !d.isLandscapeMode
    }

    RowLayout {
        anchors.fill: parent
        visible: d.isLandscapeMode

        ChatListPage {
            Layout.fillHeight: true
            Layout.preferredWidth: 240
        }

        ChatPage {
            visible: d.isChatOpened
            showBackButton: false
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        Rectangle {
            color: Theme.chatBackgroundColor
            visible: !d.isChatOpened
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    Connections {
        target: d.manager

        function onCurrentStateChanged(state) { d.onCurrentStateChanged(state) }
        function onGoBack() { d.onGoBack() }
    }
}
