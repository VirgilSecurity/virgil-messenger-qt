import QtQuick 2.15
import QtQuick.Controls 2.15

import "../base"
import "../components"
import "../components/Dialogs"
import "../theme"

Rectangle {
    color: Theme.contactsBackgroundColor

    QtObject {
        id: d
        readonly property var manager: app.stateManager
        readonly property var state: d.manager.cloudFileListState
        readonly property bool online: messenger.connectionStateString === "connected"
        readonly property int cloudFilesCount: models.cloudFiles.count
        readonly property int cloudFilesSelectedCount: models.cloudFiles.selection.selectedCount
        readonly property var cloudFolder: controllers.cloudFiles.currentFolder
    }

    Page {
        id: page
        anchors.fill: parent
        anchors.leftMargin: Theme.smallMargin
        anchors.rightMargin: Theme.smallMargin

        background: Rectangle {
            color: Theme.contactsBackgroundColor
        }

        header: SearchHeader {
            title: controllers.cloudFiles.displayPath
            description: models.cloudFiles.description
            searchPlaceholder: qsTr("Search file")
            filterSource: models.cloudFiles
            showBackButton: !controllers.cloudFiles.isRoot || d.cloudFilesSelectedCount

            ContextMenuItem {
                text: qsTr("Add files")
                onTriggered: attachmentPicker.open(AttachmentTypes.file, true)
                enabled: d.online
            }

            ContextMenuItem {
                text: qsTr("New directory")
                onTriggered: createCloudFolderDialog.open()
                visible: !d.cloudFolder.isShared
                enabled: d.online
            }

            ContextMenuItem {
                text: qsTr("Share")
                onTriggered: appState.requestSharingInfo();
                visible: d.cloudFolder.isShared
                enabled: d.online
            }

            ContextMenuSeparator {
            }

            ContextMenuItem {
                text: qsTr("Refresh")
                onTriggered: controllers.cloudFiles.refresh()
                enabled: d.online
            }

            ContextMenuSeparator {
                visible: selectAllItem.visible || deselectAllItem.visible
            }

            ContextMenuItem {
                id: selectAllItem
                text: qsTr("Select All")
                onTriggered: models.cloudFiles.selection.selectAll()
                visible: d.cloudFilesCount && (d.cloudFilesCount !== d.cloudFilesSelectedCount)
            }

            ContextMenuItem {
                id: deselectAllItem
                text: qsTr("Deselect All")
                onTriggered: models.cloudFiles.selection.clear()
                visible: d.cloudFilesSelectedCount
            }

            ContextMenuSeparator {
                visible: deleteCloudFilesItem.visible
            }

            ContextMenuItem {
                id: deleteCloudFilesItem
                text: qsTr("Delete")
                onTriggered: deleteCloudFilesDialog.open()
                visible: d.cloudFilesSelectedCount
                enabled: d.online
            }
        }

        CloudFileListView {
            searchHeader: page.header
            anchors.fill: parent
        }

        Connections {
            target: attachmentPicker

            function onPicked(fileUrls, attachmentType) {
                if (d.manager.currentState === d.state) {
                    controllers.cloudFiles.addFiles(fileUrls)
                }
            }
        }

        TransfersPanel {
            model: models.cloudFilesTransfers
            buttonVisible: !controllers.cloudFiles.isLoading
        }

        MessageDialog {
            id: deleteCloudFilesDialog
            title: qsTr("File Manager")
            text: qsTr("Delete file(s)?")
            onAccepted: controllers.cloudFiles.deleteFiles()
        }

        CreateCloudFolderDialog {
            id: createCloudFolderDialog
        }
    }
}
