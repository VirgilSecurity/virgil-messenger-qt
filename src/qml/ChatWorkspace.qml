import QtQuick 2.7
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "./chat"

Page {
    anchors.fill: parent

    RowLayout {
        id: desktopLayout
        anchors.fill: parent
        spacing: 0

        ContactPage {
            visible: isContactsShown()
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 300
            Layout.maximumWidth: isMobileView() ? root.width : 300
        }

        StackView {
            id: chatWorkspace
            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: isChatWorkspaceShown()

            property alias chatView: chatView
            initialItem: chatView

            pushEnter: null
            pushExit: null

            ConversationPage {
                id: chatView
                bgColor: "#5D6D7E"
            }

        }
    }

    function isChatWorspacePresent() {
        return chatWorkspace && chatWorkspace.depth !== 1
    }

    function isContactsShown() {
        return !isMobileView() || isMobileView() && !isChatWorspacePresent();
    }

    function isChatWorkspaceShown() {
        return !isMobileView() || isMobileView() && isChatWorspacePresent();
    }

    // Show chat with
    function showChat(contact) {
        if (contact === ConversationsModel.recipient) { return null }
        ConversationsModel.recipient = contact

        if (ConversationsModel.recipient) {
           chatWorkspace.push(Qt.createComponent("./chat/ConversationPage.qml"), { inConversationWith: contact })
        } else {
            chatWorkspace.replace(Qt.createComponent("./chat/ConversationPage.qml"), { inConversationWith: contact })
        }
    }

    function showUserSettings() {
        console.log('chatWorkspace.focus', chatWorkspace.focus)
        chatWorkspace.push(Qt.createComponent("./settings/SettingsPage.qml"))
    }
}
