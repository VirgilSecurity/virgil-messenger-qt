import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../components/CommonHelpers"
import "../theme"

Page {
    property var appState: manager.chatListState

    QtObject {
        id: d

        readonly property var manager: app.stateManager
        readonly property bool isChatList: appState === manager.chatListState
        readonly property bool isCloudFileList: appState === manager.cloudFileListState
        readonly property var cloudFilesSelection: models.cloudFiles.selection

        readonly property string chatsTitle: app.organizationDisplayName
        readonly property string chatsDescription: qsTr("%1 Server").arg(app.organizationDisplayName)

        readonly property string cloudFilesTitle: controllers.cloudFiles.displayPath
        readonly property string cloudFilesDescription: cloudFilesSelection.hasSelection ? qsTr("Selected: %1").arg(cloudFilesSelection.selectedCount) : ""
    }

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: SearchHeader {
        id: mainSearchHeader
        title: d.isChatList ? d.chatsTitle : d.cloudFilesTitle
        description: d.isChatList ? d.chatsDescription : d.cloudFilesDescription
        showDescription: description
        showBackButton: d.isCloudFileList && !controllers.cloudFiles.isRoot
        menuImage: d.isChatList ? "More" : "Plus"
        searchPlaceholder: d.isChatList ? qsTr("Search conversation") : qsTr("Search file")
        filterSource: d.isChatList ? models.chats : models.cloudFiles

        // Chat actions

        ContextMenuItem {
            text: qsTr("New chat")
            onTriggered: appState.requestNewChat()
            visible: d.isChatList
        }

        // Cloud file actions

        ContextMenuItem {
            text: qsTr("Add file")
            onTriggered: attachmentPicker.open(AttachmentTypes.file)
            visible: d.isCloudFileList
        }

        ContextMenuItem {
            text: qsTr("New directory")
            onTriggered: createCloudFolderDialog.open()
            visible: d.isCloudFileList
        }

        ContextMenuSeparator {
            visible: d.isCloudFileList
        }

        ContextMenuItem {
            text: qsTr("Refresh")
            onTriggered: controllers.cloudFiles.refresh()
            visible: d.isCloudFileList
        }

        ContextMenuSeparator {
            visible: d.isCloudFileList && d.cloudFilesSelection.hasSelection
        }

        ContextMenuItem {
            text: qsTr("Delete")
            onTriggered: deleteCloudFilesDialog.open()
            visible: d.isCloudFileList && d.cloudFilesSelection.hasSelection
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
        }

        CloudFileListView {
            searchHeader: mainSearchHeader
        }
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
            const url = fileUrls[fileUrls.length - 1]
            controllers.cloudFiles.addFile(url)
        }
    }
}
