import QtQuick 2.12

import "../components"

OperationPage {
    state: app.stateManager.signUpState
    loadingText: qsTr("Registering %1...").arg(state.userId)

    readonly property string terms:
        "By creating an account, you agree to Virgil's " +
        "<a style='text-decoration: none; color: %1' href='https://virgilsecurity.com/terms-of-service/'>Terms of Service</a> " +
        "and <a style='text-decoration: none; color: %1' href='https://virgilsecurity.com/privacy-policy/'>Privacy Policy</a>"

    header: Header {
        title: qsTr("Register")
        showBackButton: !form.isLoading
    }

    Form {
        id: form

        UserNameFormInput {
            id: username
            placeholder: qsTr('Username')
        }

        FormPrimaryButton {
            onClicked: app.stateManager.signUp(username.text)
            text: qsTr("Create account")
        }

        FormLabel {
            text: terms.arg(Theme.buttonPrimaryColor)
        }
    }
}
