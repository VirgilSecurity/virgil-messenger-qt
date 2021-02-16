import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"
import "../components"

Page {
    readonly property var appState: app.stateManager.chatState

    QtObject {
        id: d
        readonly property var chat: controllers.chats.current
        readonly property var model: chat.contacts
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        title: d.chat.isGroup ? qsTr("Group info") : qsTr("Chat info")
    }

    ColumnLayout {
        anchors {
            fill: parent
            leftMargin: Theme.smallMargin
            rightMargin: Theme.smallMargin
            topMargin: Theme.margin
            bottomMargin: Theme.margin
        }
        spacing: Theme.smallSpacing

        Row {
            spacing: Theme.spacing
            Layout.leftMargin: Theme.smallMargin

            Avatar {
                nickname: d.chat.title
            }

            HeaderTitle {
                title: d.chat.title
                description: qsTr("%1 members").arg(10)
            }
        }

        Item {
            height: Theme.smallSpacing
        }

        Text {
            text: qsTr("Participants")
            color: "white"
            Layout.leftMargin: Theme.smallMargin
        }

        Row {
            spacing: Theme.smallSpacing

            FormPrimaryButton {
                text: qsTr("Add")
            }

            FormPrimaryButton {
                text: qsTr("Remove")
                visible: d.model.selection.hasSelection
                onClicked: controllers.chats.removeSelectedMembers()
            }
        }

        ContactsListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: d.model.proxy
            onContactSelected: d.model.toggleByUsername(contactUsername)
        }
    }
}
