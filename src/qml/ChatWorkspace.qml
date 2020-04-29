import QtQuick 2.7
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "./chat"

Page {
    Loader {
        id: loader
        anchors.fill: parent
        source: isMobileView() ? "./chat/ContactPage.qml" : "./chat/DesktopChat.qml"
    }

    // Show chat with
    function showChat(contact) {
        ConversationsModel.recipient = contact

        // Mobile
        if (isMobileView()) {
            screenManager.replace("./chat/ConversationPage.qml", { inConversationWith: contact })
        } else {
            // Desktop
            loader.item.chatView.inConversationWith = contact
        }
    }
}
