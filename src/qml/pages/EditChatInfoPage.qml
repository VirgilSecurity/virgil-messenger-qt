import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

OperationPage {
    id: root

    appState: app.stateManager.editChatInfoState

    signal saved()

    QtObject {
        id: d
        readonly property var chat: controllers.chats.current
    }

    header: Header { title: qsTr("Edit group") }

    Form {
        id: form

        Avatar {
            nickname: d.chat.title
            diameter: Theme.bigAvatarDiameter
            Layout.alignment: Qt.AlignHCenter
        }

        FormInput {
            id: groupNameInput
            label: qsTr("Group name")
            placeholder: qsTr("Enter group name")
            text: d.chat.title
            Layout.alignment: Qt.AlignHCenter
        }

        FormPrimaryButton {
            onClicked: root.appState.save(groupNameInput.text)
            text: qsTr("Save")
            enabled: groupNameInput.text && groupNameInput.text !== d.chat.title
        }
    }

    Component.onCompleted: appState.saved.connect(saved)
}
