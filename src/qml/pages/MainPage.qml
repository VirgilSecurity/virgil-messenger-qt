import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

Page {
    id: root

    signal accountSettingsRequested()
    signal newChatRequested()
    signal newGroupChatRequested()
    signal chatInfoRequested()
    signal cloudFileSharingRequested()
    signal cloudFileSetMembersRequested(string name)

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SidebarPanel {
            id: sidebarPanel
            Layout.preferredWidth: Theme.headerHeight
            Layout.fillHeight: true

            onAccountSettingsRequested: root.accountSettingsRequested()
        }

        StackLayout {
            id: stackLayout
            clip: true
            currentIndex: sidebarPanel.buttonIndex
            Layout.fillWidth: true
            Layout.fillHeight: true

            ChatPages {
                id: chatPages
                onNewChatRequested: root.newChatRequested()
                onNewGroupChatRequested: root.newGroupChatRequested()
                onInfoRequested: root.chatInfoRequested()
            }

            CloudFileListPage {
                onNewFolderRequested: controllers.cloudFiles.createFolder(name, [])
                onNewSharedFolderRequested: root.cloudFileSetMembersRequested(name)
                onSharingRequested: root.cloudFileSharingRequested()
            }

            function navigateBack(transition) { return itemAt(currentIndex).navigateBack(transition) }
            function currentPage() { return itemAt(currentIndex).currentPage() }

            onCurrentIndexChanged: window.updateAppState()
        }
    }

    Connections {
        target: attachmentPicker

        function onPicked(fileUrls, attachmentType) {
            if (sidebarPanel.buttonIndex == 0) {
                const url = fileUrls[fileUrls.length - 1]
                if (attachmentType === AttachmentTypes.picture) {
                    controllers.messages.sendPictureMessage(url, attachmentType)
                }
                else {
                    controllers.messages.sendFileMessage(url, attachmentType)
                }
            }
            else {
                controllers.cloudFiles.addFiles(fileUrls)
            }
        }
    }

    function navigateBack(transition) { return stackLayout.navigateBack(transition) }
    function currentPage() { return stackLayout.currentPage() }

    Component.onCompleted: window.updateAppState()
}
