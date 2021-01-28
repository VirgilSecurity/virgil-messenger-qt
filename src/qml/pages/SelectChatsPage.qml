import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

Page {
    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: NewGroupChatHeader {
        id: selectChatsHeader
        title: qsTr("Share to...")
        description: d.selectedCount > 0 ? qsTr("%1 selected").arg(d.selectedCount) : ""
        rightButtonEnabled: d.selectedCount > 0

        onActionButtonClicked: {
            controllers.messages.shareMessage(app.stateManager.chatState.currentMessageId)
            app.stateManager.goBack()
        }
    }

    QtObject {
        id: d
        readonly property var model: models.chats
        readonly property int selectedCount: d.model.selection.selectedCount
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
            model: d.model.proxy

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
