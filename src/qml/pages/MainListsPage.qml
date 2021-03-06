import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../components/CommonHelpers"
import "../components/Dialogs"
import "../theme"

Page {
    property var appState: manager.chatListState

    QtObject {
        id: d

        readonly property var manager: app.stateManager
        readonly property bool isChatList: appState === manager.chatListState
        readonly property bool isCloudFileList: appState === manager.cloudFileListState
        readonly property int cloudFilesCount: models.cloudFiles.count
        readonly property int cloudFilesSelectedCount: models.cloudFiles.selection.selectedCount
        readonly property var cloudFolder: controllers.cloudFiles.currentFolder

        readonly property string chatsTitle: app.organizationDisplayName
        readonly property string chatsDescription: qsTr("%1 Server").arg(app.organizationDisplayName)

        readonly property string cloudFilesTitle: controllers.cloudFiles.displayPath
        readonly property string cloudFilesDescription: models.cloudFiles.description
    }

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: SearchHeader {
        id: mainSearchHeader
        title: d.isChatList ? d.chatsTitle : d.cloudFilesTitle
        description: d.isChatList ? d.chatsDescription : d.cloudFilesDescription
        showBackButton: d.isCloudFileList && !controllers.cloudFiles.isRoot
        searchPlaceholder: d.isChatList ? qsTr("Search conversation") : qsTr("Search file")
        filterSource: d.isChatList ? models.chats : models.cloudFiles

        // Chat actions

        ContextMenuItem {
            text: qsTr("New chat")
            onTriggered: appState.requestNewChat()
            visible: d.isChatList
        }

        ContextMenuItem {
            text: qsTr("New group")
            onTriggered: appState.requestNewGroupChat()
            visible: d.isChatList
        }

        // Cloud file actions

        ContextMenuItem {
            text: qsTr("Add files")
            onTriggered: attachmentPicker.open(AttachmentTypes.file, true)
            visible: d.isCloudFileList
        }

        ContextMenuItem {
            text: qsTr("New directory")
            onTriggered: createCloudFolderDialog.open()
            visible: d.isCloudFileList && !d.cloudFolder.isShared
        }

        ContextMenuItem {
            text: qsTr("Sharing")
            onTriggered: appState.requestSharingInfo();
            visible: d.isCloudFileList && d.cloudFolder.isShared
        }

        ContextMenuSeparator {
            visible: d.isCloudFileList
        }

        ContextMenuItem {
            text: qsTr("Refresh")
            onTriggered: controllers.cloudFiles.refresh()
            visible: d.isCloudFileList
        }

        ContextMenuItem {
            text: qsTr("Select All")
            onTriggered: models.cloudFiles.selection.selectAll()
            visible: d.isCloudFileList && d.cloudFilesCount && (d.cloudFilesCount !== d.cloudFilesSelectedCount)
        }

        ContextMenuSeparator {
            visible: deleteCloudFilesItem.visible
        }

        ContextMenuItem {
            id: deleteCloudFilesItem
            text: qsTr("Delete")
            onTriggered: deleteCloudFilesDialog.open()
            visible: d.isCloudFileList && d.cloudFilesSelectedCount
        }
    }

    StackLayout {
        anchors {
            leftMargin: Theme.smallMargin
            rightMargin: Theme.smallMargin
            fill: parent
        }
        currentIndex: d.isChatList ? 0 : 1

        ChatListView {
            searchHeader: mainSearchHeader
            onChatSelected: controllers.chats.openChat(chatId)
        }

        CloudFileListView {
            searchHeader: mainSearchHeader
        }
    }

    TransfersPanel {
        model: models.cloudFilesTransfers
        visible: d.isCloudFileList
        buttonVisible: !controllers.cloudFiles.isLoading
    }

    Connections {
        target: d.manager

        function onCurrentStateChanged(state) {
            if ([d.manager.chatListState, d.manager.cloudFileListState].includes(state)) {
                appState = state
            }
        }
    }

    Connections {
        target: attachmentPicker

        function onPicked(fileUrls, attachmentType) {
            if (d.manager.currentState !== d.manager.cloudFileListState) {
                return;
            }
            controllers.cloudFiles.addFiles(fileUrls)
        }
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
