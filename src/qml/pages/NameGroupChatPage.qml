import QtQuick 2.12

import "../components"

OperationPage {
    appState: app.stateManager.nameGroupChatState
    loadingText: qsTr("Creating new group...")

    header: Header {
        showBackButton: !form.isLoading
        title: qsTr("Create group")
    }

    Form {
        id: form

        FormInput {
            id: password
            label: qsTr("Group name")
            password: false
            placeholder: qsTr("Enter name")
        }

        FormPrimaryButton {
            onClicked: {}
            text: qsTr("Create")
        }
    }
}
