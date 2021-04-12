import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../components/Dialogs"
import "../theme"

Page {
    readonly property var appState: app.stateManager.chatInfoState

    QtObject {
        id: d
        readonly property var chat: controllers.chats.current
        readonly property var model: chat.groupMembers
        readonly property bool editableMembership: false
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        id: pageHeader
        title: d.chat.isGroup ? qsTr("Group info") : qsTr("Chat info")
        contextMenuVisible: d.chat.isGroup && (d.groupMembersEditable || !d.isOwnGroup)
        contextMenu: ContextMenu {
            ContextMenuItem {
                text: d.chat.isGroup ? qsTr("Edit group") : qsTr("Edit chat")
                visible: d.chat.isGroup && d.chat.userCanEdit
                onTriggered: appState.editRequested()
            }

            ContextMenuItem {
                id: addMembersItem
                text: qsTr("Add members")
                visible: d.chat.isGroup && d.chat.userCanEdit && d.editableMembership
                onTriggered: appState.addMembersRequested()
            }

            ContextMenuSeparator {
                visible: addMembersItem.visible && removeMembersItem.visible
            }

            ContextMenuItem {
                id: removeMembersItem
                text: qsTr("Remove members")
                visible: d.chat.isGroup && d.model.selection.hasSelection && d.editableMembership
                onTriggered: removeParticipantsDialog.open()
            }

            ContextMenuItem {
                id: leaveGroupItem
                text: qsTr("Leave group")
                visible: d.chat.isGroup && !d.chat.userIsOwner && d.editableMembership
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
                clickable: d.chat.isGroup && d.chat.userCanEdit
                onClicked: appState.editRequested()
            }
        }

        TabView {
            visible: d.chat.isGroup
            Layout.topMargin: Theme.margin

            ContactsListView {
                readonly property var tabTitle: qsTr("Participants")
                model: d.model.proxy
                selectionModel: d.chat.isGroup && d.chat.userCanEdit && d.editableMembership ? d.model.selection : null
                itemContextMenu: ContextMenu {
                    dropdown: true

                    ContextMenuItem {
                        text: qsTr("Remove member")
                        onTriggered: removeParticipantsDialog.open()
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            visible: !d.chat.isGroup
        }
    }

    MessageDialog {
        id: removeParticipantsDialog
        title: pageHeader.title
        text: qsTr("Remove participant(s)?")
        onAccepted: controllers.chats.removeSelectedMembers()
    }
}
