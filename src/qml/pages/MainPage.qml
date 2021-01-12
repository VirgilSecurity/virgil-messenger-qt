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
        readonly property var fileCloudSelection: models.fileCloud.selection

        readonly property string chatTitle: app.organizationDisplayName
        readonly property string chatDescription: qsTr("%1 Server").arg(app.organizationDisplayName)

        readonly property string fileCloudTitle: qsTr("File Cloud") + controllers.fileCloud.displayPath
        readonly property string fileCloudDescription: fileCloudSelection.hasSelection ? qsTr("Selected: %1").arg(fileCloudSelection.selectedCount) : " "
    }

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: SearchHeader {
        id: mainSearchHeader
        title: d.isChatList ? d.chatTitle : d.fileCloudTitle
        description: d.isChatList ? d.chatDescription : d.fileCloudDescription
        showBackButton: !d.isChatList && controllers.fileCloud.displayPath
        menuImage: d.isChatList ? "More" : "Plus"
        searchPlaceholder: d.isChatList ? qsTr("Search conversation") : qsTr("Search file")
        filterSource: d.isChatList ? models.chats : models.fileCloud

        Action {
            text: d.isChatList ? qsTr("New chat") : qsTr("Add file")
            onTriggered: d.isChatList ? appState.requestNewChat() : attachmentPicker.open(AttachmentTypes.file)
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

        FileCloudListView {
            searchHeader: mainSearchHeader
        }
    }

    Connections {
        target: d.manager

        function onCurrentStateChanged(state) {
            if ([d.manager.chatListState, d.manager.fileCloudState].includes(state)) {
                appState = state
            }
        }
    }

    Connections {
        target: attachmentPicker

        function onPicked(fileUrls, attachmentType) {
            if (d.manager.currentState !== d.manager.fileCloudState) {
                return;
            }
            const url = fileUrls[fileUrls.length - 1]
            controllers.fileCloud.addFile(url)
        }
    }
}
