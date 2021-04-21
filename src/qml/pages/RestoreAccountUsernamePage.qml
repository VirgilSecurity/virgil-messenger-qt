import QtQuick 2.15

import "../components"

OperationPage {
    id: root

    appState: app.stateManager.restoreAccountUsernameState
    loadingText: qsTr("Loading...")

    signal usernameValidated(string username)

    header: Header {
        showBackButton: !form.isLoading
        title: qsTr("Sign In")
    }

    Form {
        id: form

        UserNameFormInput {
            id: username
            placeholder: qsTr("Enter Username")
        }

        FormPrimaryButton {
            onClicked: appState.validate(username.text)
            text: qsTr("Sign In")
        }
    }

    Component.onCompleted: appState.validated.connect(root.usernameValidated)
}
