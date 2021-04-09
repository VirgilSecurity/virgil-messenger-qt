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
                    stackView.push(chatComponent, StackView.Transition)
                }
            }
        }

        function onGoBack() {
            if (manager.currentState === manager.chatState) {
                stackView.pop()
            }
        }
    }

    Component {
        id: chatListComponent
        ChatListPage {}
    }

    Component {
        id: chatComponent
        ChatPage {}
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: chatListComponent
        visible: !d.isLandscapeMode
    }

    RowLayout {
        anchors.fill: parent
        visible: d.isLandscapeMode

        Loader {
            sourceComponent: chatListComponent
            Layout.fillHeight: true
            Layout.preferredWidth: 240
        }

        Loader {
            sourceComponent: chatComponent
            visible: d.isChatOpened
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