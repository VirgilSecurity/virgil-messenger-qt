import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

Page {
    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: Header {
        title: qsTr("Share to...")
    }

    Form {
        isCentered: false

        Search {
            state: "opened"
            searchPlaceholder: qsTr("Search chat")
            textValidator: app.validator.reUsername
            closeable: false

            Layout.preferredHeight: recommendedHeight
            Layout.fillWidth: true

            onSearchChanged: models.chats.filter = search
        }

        ChatListView {
            id: listView
            model: models.chats.proxy

            Layout.fillWidth: true
            Layout.fillHeight: true

            onChatSelected: {
                controllers.messages.shareMessage(app.stateManager.chatState.currentMessageId, chatId)
                app.stateManager.goBack()
            }
        }
    }
}
