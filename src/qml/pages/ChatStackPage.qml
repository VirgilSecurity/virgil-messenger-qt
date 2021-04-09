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
        readonly property bool isChatOpened: !controllers.chats.current.isNull

        readonly property real listMinimumWidth: 240
        readonly property real listMaximumWidth: 400
        readonly property real chatMinimumWidth: 500
        readonly property real splitHandleSize: 4
        readonly property bool isLandscapeMode: Platform.isDesktop && width >= listMinimumWidth + splitHandleSize + chatMinimumWidth

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

    SplitView {
        id: splitView
        anchors.fill: parent
        visible: d.isLandscapeMode

        handle: Rectangle {
            implicitWidth: d.splitHandleSize
            implicitHeight: d.splitHandleSize
            color: Theme.chatBackgroundColor
        }

        ChatListPage {
            SplitView.fillHeight: true
            SplitView.fillWidth: false
            SplitView.preferredWidth: d.listMinimumWidth
            SplitView.minimumWidth: d.listMinimumWidth
            SplitView.maximumWidth: Math.min(d.listMaximumWidth, splitView.width - d.chatMinimumWidth - d.splitHandleSize)
        }

        ChatPage {
            visible: d.isChatOpened
            showBackButton: false
            SplitView.minimumWidth: d.chatMinimumWidth
            SplitView.fillHeight: true
            SplitView.fillWidth: true
        }

        Rectangle {
            visible: !d.isChatOpened
            color: Theme.chatBackgroundColor
            SplitView.minimumWidth: d.chatMinimumWidth
            SplitView.fillHeight: true
            SplitView.fillWidth: true
        }
    }

    Connections {
        target: d.manager

        function onCurrentStateChanged(state) { d.onCurrentStateChanged(state) }
        function onGoBack() { d.onGoBack() }
    }
}
