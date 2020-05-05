import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../pages"

Page {
    id: chatView
    focus: true
    property var currentContact: ConversationsModel.recipient

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
            id: chatLayoutStack
            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: isChatViewStackShown()

            initialItem: conversationPage

            ConversationPage {
                id: conversationPage
                inConversationWith: currentContact
            }
        }
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
            event.accepted = back()
        }
    }

    function back() {
        if (chatLayoutStack.depth > 1) {
            chatLayoutStack.pop(StackView.Immediate)
            return true
        } else if (currentContact) {
            showContacts()
            return true
        } else {
            return false
        }
    }

    function isChatSelected() {
        return chatLayoutStack.depth > 1 || currentContact
    }

    function isContactsShown() {
        return !isMobileView() || isMobileView() && !isChatSelected();
    }

    function isChatViewStackShown() {
        return !isMobileView() || isMobileView() && isChatSelected();
    }

    function showChat(contact) {
        if (contact === ConversationsModel.recipient) { return null }
        ConversationsModel.recipient = contact
        chatLayoutStack.pop(null, StackView.Immediate)
    }

    function showContacts() {
        showChat(null)
    }

    function showUserSettings() {
        chatLayoutStack.push(Qt.createComponent("../pages/AccountSettingsPage.qml"), StackView.Immediate)
    }
}
