import QtQuick 2.12

import "../components"

OperationPage {
    appState: app.stateManager.nameGroupChatState
    loadingText: qsTr("Creating new group...")

    header: Header {
        showBackButton: !form.isLoading
        title: qsTr("New group")
    }

    Form {
        id: form

        FormInput {
            id: nameInput
            label: qsTr("Group name")
            placeholder: qsTr("Enter name")
            // TODO(fpohtmeh): validate
        }

        FormPrimaryButton {
            onClicked: appState.groupNamed(nameInput.text)
            text: qsTr("Select members")
        }
    }
}
