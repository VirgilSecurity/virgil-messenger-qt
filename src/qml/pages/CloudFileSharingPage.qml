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
        readonly property bool groupMembersEditable: true
        readonly property bool isOwnFile: true
        readonly property int selectedGroupMembersCount: model.selection.selectedCount
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        id: pageHeader
        title: qsTr("Cloud folder info")
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
                onTriggered: removeParticipantsDialog.open()
            }

            ContextMenuItem {
                text: qsTr("Leave")
                visible: !d.isOwnFile
                onTriggered: controllers.cloudFiles.leaveMembership()
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
                readonly property var tabTitle: d.selectedGroupMembersCount
                                                ? qsTr("Participants (%1 selected)").arg(d.selectedGroupMembersCount)
                                                : qsTr("Participants")
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
