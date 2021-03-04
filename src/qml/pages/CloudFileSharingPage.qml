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
        readonly property var cloudFile: controllers.cloudFiles.current
        readonly property var model: cloudFile.members
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        id: pageHeader
        title: qsTr("Cloud folder info")
        contextMenu: ContextMenu {
            ContextMenuItem {
                id: addMembersItem
                text: qsTr("Add members")
                visible: d.cloudFile.userIsOwner
                onTriggered: appState.addMembersRequested()
            }

            ContextMenuSeparator {
                visible: addMembersItem.visible && removeMembersItem.visible
            }

            ContextMenuItem {
                id: removeMembersItem
                text: qsTr("Remove members")
                visible: d.model.selection.hasSelection
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
            model: d.cloudFile.properties
        }

        TabView {
            Layout.topMargin: Theme.margin
            Layout.fillHeight: true

            ContactsListView {
                readonly property var tabTitle: qsTr("Participants")
                model: d.model.proxy
                onContactSelected: d.model.toggleByUsername(contactUsername)
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
