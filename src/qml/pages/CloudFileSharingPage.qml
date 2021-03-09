import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"
import "../components"
import "../components/Dialogs"

Page {
    readonly property var appState: app.stateManager.cloudFileSharingState

    QtObject {
        id: d
        readonly property var cloudFolder: controllers.cloudFiles.currentFolder
        readonly property var model: cloudFolder.members
        readonly property var selection: model.selection
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        id: pageHeader
        title: qsTr("Cloud folder info")
        contextMenuVisible: d.cloudFolder.userIsOwner
        contextMenu: ContextMenu {
            ContextMenuItem {
                id: addMembersItem
                text: qsTr("Add members")
                visible: d.cloudFolder.userIsOwner
                onTriggered: appState.addMembersRequested()
            }

            ContextMenuSeparator {
                visible: addMembersItem.visible && removeMembersItem.visible
            }

            ContextMenuItem {
                id: removeMembersItem
                text: qsTr("Remove members")
                visible: d.cloudFolder.userIsOwner && d.model.selection.hasSelection
                onTriggered: removeParticipantsDialog.open()
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

        PropertiesView {
            Layout.leftMargin: Theme.smallMargin
            model: d.cloudFolder.properties
        }

        TabView {
            Layout.topMargin: Theme.margin
            Layout.fillHeight: true

            ContactsListView {
                readonly property var tabTitle: qsTr("Participants")
                model: d.model.proxy
                selectionModel: d.cloudFolder.userIsOwner ? d.selection : null
                itemContextMenu: ContextMenu {
                    dropdown: true

                    ContextMenuItem {
                        text: qsTr("Remove member")
                        onTriggered: removeParticipantsDialog.open()
                    }
                }
            }
        }
    }

    MessageDialog {
        id: removeParticipantsDialog
        title: pageHeader.title
        text: qsTr("Remove participant(s)?")
        onAccepted: controllers.cloudFiles.removeSelectedMembers()
    }
}
