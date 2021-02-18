import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"
import "../components"
import "../components/Dialogs"

Page {
    readonly property var appState: app.stateManager.chatInfoState

    QtObject {
        id: d
        readonly property var chat: controllers.chats.current
        readonly property var model: chat.groupMembers
        readonly property bool groupMembersEditable: chat.isGroup && !model.isReadOnly
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        title: d.chat.isGroup ? qsTr("Group info") : qsTr("Chat info")
        contextMenuVisible: d.groupMembersEditable
        contextMenu: ContextMenu {
            ContextMenuItem {
                text: qsTr("Add members")
                visible: d.groupMembersEditable
                onTriggered: appState.addMembersRequested()
            }

            ContextMenuSeparator {
                visible: d.groupMembersEditable && d.model.selection.hasSelection
            }

            ContextMenuItem {
                text: qsTr("Remove members")
                visible: d.groupMembersEditable && d.model.selection.hasSelection
                onTriggered: deleteGroupMembersDialog.open()
            }
        }
    }

    ColumnLayout {
        anchors {
            fill: parent
            leftMargin: Theme.smallMargin
            rightMargin: Theme.smallMargin
            topMargin: Theme.margin
            bottomMargin: Theme.margin
        }
        spacing: Theme.smallSpacing

        Row {
            spacing: Theme.spacing
            Layout.leftMargin: Theme.smallMargin

            Avatar {
                nickname: d.chat.title
            }

            HeaderTitle {
                title: d.chat.title
                description: qsTr("%1 members").arg(10)
            }
        }

        Item {
            height: Theme.smallSpacing
        }

        Text {
            text: qsTr("Participants")
            color: "white"
            Layout.leftMargin: Theme.smallMargin
        }

        ContactsListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: d.model.proxy
            isSelectable: d.groupMembersEditable
            onContactSelected: d.model.toggleByUsername(contactUsername)
        }
    }

    MessageDialog {
        id: deleteGroupMembersDialog
        title: qsTr("Group")
        text: qsTr("Remove group members(s)?")
        onAccepted: controllers.chats.removeSelectedMembers()
    }
}
