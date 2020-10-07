import QtQuick 2.12

import "../components"

OperationPage {
    state: app.stateManager.signInUsernameState
    loadingText: qsTr("Loading...")

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
            onClicked: app.stateManager.openSignInAs(username.text)
            text: qsTr("Sign In")
        }
    }
}
