import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

Page {
    QtObject {
        id: d
        readonly property var manager: app.stateManager

        function updateStackIndex(state) {
            if (state === manager.chatListState) {
                stackLayout.currentIndex = 0
            }
            else if (state === manager.cloudFileListState) {
                stackLayout.currentIndex = 1
            }
        }
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
            id: stackLayout
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true

            ChatStackPage {
            }

            CloudFileListPage {
            }
        }
    }

    Connections {
        target: d.manager

        function onCurrentStateChanged(state) {
            d.updateStackIndex(state)
        }
    }
}
