import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import com.virgilsecurity.messenger 1.0

import "../theme"
import "../components"
import "../components/CommonHelpers"

Page {
    property var appState: manager.chatListState

    readonly property var manager: app.stateManager
    readonly property bool isChatList: appState === manager.chatListState

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: SearchHeader {
        id: searchHeader
        title: isChatList ? app.organizationDisplayName : qsTr("File Cloud") + controllers.fileCloud.displayPath
        description: isChatList ? qsTr("%1 Server").arg(app.organizationDisplayName) : ""
        showDescription: isChatList
        showBackButton: !isChatList && controllers.fileCloud.displayPath
        menuImage: isChatList ? "More" : "Plus"
        searchPlaceholder: isChatList ? qsTr("Search conversation") : qsTr("Search file")
        filterSource: isChatList ? models.chats : models.fileCloud

        Action {
            text: isChatList ? qsTr("New chat") : qsTr("Add file")
            onTriggered: isChatList ? appState.requestNewChat() : attachmentPicker.open(Enums.AttachmentType.File)
        }

        Action {
            text: qsTr("New group chat")
            enabled: isChatList
            onTriggered: appState.requestNewGroupChat()
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: isChatList ? 0 : 1

        ModelListView {
            id: chatListView
            model: models.chats.proxy
            searchHeader: searchHeader

            delegate: ListDelegate {
                width: chatListView.width

                Avatar {
                    id: avatar
                    nickname: model.contactId
                }

                Column {
                    Layout.fillWidth: true
                    clip: true

                    Text {
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(15)
                        text: model.contactId
                    }

                    Text {
                        id: messageBody
                        color: Theme.secondaryTextColor
                        font.pointSize: UiHelper.fixFontSz(12)
                        width: parent.width
                        text: model.lastMessageBody
                        elide: Text.ElideRight
                    }
                }

                Column {
                    width: 30
                    spacing: 5

                    MessageCounter {
                       count: model.unreadMessageCount
                       anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: model.lastEventTime
                        color: Theme.secondaryTextColor
                        font.pointSize: UiHelper.fixFontSz(9)
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                onClicked: controllers.chats.openChatById(model.id)
            }

            onPlaceholderClicked: appState.requestNewChat()
        }

        ModelListView {
            id: fileCloudListView
            model: models.fileCloud.proxy
            searchHeader: searchHeader
            emptyText: qsTr("Add a file<br/>by pressing the plus<br/>button above")

            delegate: ListDelegate {
                width: fileCloudListView.width

                ImageButton {
                    image: model.isDir ? "Folder-Big" : "File-Big"
                    imageSize: 48
                    iconSize: 40
                    onClicked: controllers.fileCloud.processClick(index)
                }

                Column {
                    Layout.fillWidth: true
                    clip: true

                    Text {
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(15)
                        text: model.fileName
                        width: parent.width
                    }
                }

                Column {
                    width: 30
                    spacing: 5

                    Text {
                        text: model.displayFileSize
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(12)
                        anchors.right: parent.right
                    }

                    Text {
                        text: model.displayDateTime
                        color: Theme.secondaryTextColor
                        font.pointSize: UiHelper.fixFontSz(9)
                        anchors.right: parent.right
                    }
                }

                onClicked: controllers.fileCloud.processClick(index)
            }

            onPlaceholderClicked: attachmentPicker.open(Enums.AttachmentType.File)
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
