import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

AuthorizationPage {
    authorizationState: app.stateManager.signInState

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
