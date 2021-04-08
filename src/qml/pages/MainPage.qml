import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

Page {
    QtObject {
        id: d
        readonly property var manager: app.stateManager
        readonly property bool isChatList: d.manager.currentState === manager.chatListState
    }

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SidebarPanel {
            Layout.preferredWidth: Theme.headerHeight
            Layout.fillHeight: true
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: Theme.smallMargin
            Layout.rightMargin: Theme.smallMargin
            currentIndex: d.isChatList ? 0 : 1

            ChatListPage {
            }

            CloudFileListPage {
            }
        }
    }
}
