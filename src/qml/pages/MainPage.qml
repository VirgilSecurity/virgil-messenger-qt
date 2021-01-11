import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../components/CommonHelpers"
import "../theme"

Page {
    property var appState: manager.chatListState

    readonly property var manager: app.stateManager
    readonly property bool isChatList: appState === manager.chatListState

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: SearchHeader {
        id: mainSearchHeader
        title: isChatList ? app.organizationDisplayName : qsTr("File Cloud") + controllers.fileCloud.displayPath
        description: isChatList ? qsTr("%1 Server").arg(app.organizationDisplayName) : qsTr("Selected: %1").arg(models.fileCloud.selection.selectedCount)
        showDescription: isChatList || models.fileCloud.selection.hasSelection
        showBackButton: !isChatList && controllers.fileCloud.displayPath
        menuImage: isChatList ? "More" : "Plus"
        searchPlaceholder: isChatList ? qsTr("Search conversation") : qsTr("Search file")
        filterSource: isChatList ? models.chats : models.fileCloud

        Action {
            text: isChatList ? qsTr("New chat") : qsTr("Add file")
            onTriggered: isChatList ? appState.requestNewChat() : attachmentPicker.open(AttachmentTypes.file)
        }

        // TODO(fpohtmeh): restore later
//        Action {
//            text: qsTr("New group")
//            enabled: isChatList
//            onTriggered: appState.requestNewGroupChat()
//        }
    }

    StackLayout {
        anchors {
            leftMargin: Theme.smallMargin
            rightMargin: Theme.smallMargin
            fill: parent
        }
        currentIndex: isChatList ? 0 : 1

        ChatListView {
            searchHeader: mainSearchHeader
        }

        FileCloudListView {
            searchHeader: mainSearchHeader
        }
    }

    Connections {
        target: app.stateManager

        function onCurrentStateChanged(state) {
            if ([manager.chatListState, manager.fileCloudState].includes(state)) {
                appState = state
            }
        }
    }

    Connections {
        target: attachmentPicker

        function onPicked(fileUrls, attachmentType) {
            if (manager.currentState !== manager.fileCloudState) {
                return;
            }
            const url = fileUrls[fileUrls.length - 1]
            controllers.fileCloud.addFile(url)
        }
    }
}
