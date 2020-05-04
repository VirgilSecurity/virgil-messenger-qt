import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "./chat"

Page {
    property var currentContact: ConversationsModel.recipient

    Keys.onEscapePressed: back()

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
            id: chatWorkspaceStack
            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: isChatWorkspaceShown()

            property alias chatView: chatView
            initialItem: chatView

            ConversationPage {
                id: chatView
                bgColor: "#5D6D7E"
                inConversationWith: currentContact
            }

        }
    }

    function isChatWorskpacePresent() {
        return chatWorkspaceStack.depth > 1 || currentContact
    }

    function isContactsShown() {
        return !isMobileView() || isMobileView() && !isChatWorskpacePresent();
    }

    function isChatWorkspaceShown() {
        return !isMobileView() || isMobileView() && isChatWorskpacePresent();
    }

    function showChat(contact) {
        if (contact === ConversationsModel.recipient) { return null }
        ConversationsModel.recipient = contact
    }

    function showContacts() {
        showChat(null)
    }

    function showUserSettings() {
        chatWorkspaceStack.push(Qt.createComponent("./settings/SettingsPage.qml"), StackView.Immediate)
    }

    function back() {
        if (chatWorkspaceStack.depth > 1) {
            chatWorkspaceStack.pop(StackView.Immediate)
        } else {
            showContacts()
        }
    }
}
