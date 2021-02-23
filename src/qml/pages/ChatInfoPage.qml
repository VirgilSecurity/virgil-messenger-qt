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
        // FIXME(fpohtmeh): restore once it's implemented in core
        readonly property bool groupMembersEditable: false // chat.isGroup && !model.isReadOnly
        readonly property bool isOwnGroup: chat.isGroup // chat.isGroup && model.isOwnedByUser
        readonly property int selectedGroupMembersCount: model.selection.selectedCount
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        title: d.chat.isGroup ? qsTr("Group info") : qsTr("Chat info")
        contextMenuVisible: d.groupMembersEditable || !d.isOwnGroup
        contextMenu: ContextMenu {
            ContextMenuItem {
                text: qsTr("Add members")
                visible: d.groupMembersEditable
                onTriggered: appState.addMembersRequested()
            }

            ContextMenuSeparator {
                visible: d.groupMembersEditable && d.selectedGroupMembersCount
            }

            ContextMenuItem {
                text: qsTr("Remove members")
                visible: d.groupMembersEditable && d.selectedGroupMembersCount
                onTriggered: deleteGroupMembersDialog.open()
            }

            ContextMenuItem {
                text: qsTr("Leave group")
                visible: !d.isOwnGroup
                onTriggered: controllers.chats.leaveGroup()
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
                description: d.chat.isGroup ? qsTr("%1 members").arg(d.model.count) : d.chat.lastActivityText
            }
        }

        Item {
            height: Theme.smallSpacing
        }

        Text {
            Layout.leftMargin: Theme.smallMargin
            text: d.selectedGroupMembersCount ? qsTr("Participants (%1 selected)").arg(d.selectedGroupMembersCount) : qsTr("Participants")
            color: "white"
            visible: d.chat.isGroup
        }

        ContactsListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: d.model.proxy
            isSelectable: d.groupMembersEditable
            visible: d.chat.isGroup
            onContactSelected: d.model.toggleByUsername(contactUsername)
        }

        Item {
            Layout.fillHeight: true
            visible: !d.chat.isGroup
        }
    }

    MessageDialog {
        id: deleteGroupMembersDialog
        title: qsTr("Group")
        text: qsTr("Remove group members(s)?")
        onAccepted: controllers.chats.removeSelectedMembers()
    }
}
