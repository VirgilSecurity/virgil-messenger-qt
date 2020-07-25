import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0

import "../theme"
import "../components"
import "../helpers/login.js" as LoginLogic

Page {
    readonly property string terms:
        "By creating an account, you agree to Virgil's " +
        "<a style='text-decoration: none; color: %1' href='https://virgilsecurity.com/terms-of-service/'>Terms of Service</a> " +
        "and <a style='text-decoration: none; color: %1' href='https://virgilsecurity.com/privacy-policy/'>Privacy Policy</a>"

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    header: Header {
        title: "Register"
        showBackButton: !form.isLoading
    }

    Form {
        id: form

        UserNameFormInput {
            id: username
            placeholder: qsTr('Username')
        }

        FormPrimaryButton {
            onClicked: messenger.signUp(username.text)
            objectName: "btnCreateAccount"
            text: "Create account"
        }

        FormLabel {
            text: terms.arg(Theme.buttonPrimaryColor)
        }
    }

    footer: Footer {}

    Connections {
        target: messenger
        onSignUp: form.showLoading(qsTr("Registering %1...".arg(userWithEnv)))
        onSignedUp: {
            form.hideLoading()
            mainView.showContacts("Contacts", null, true, true) // TODO(fpohtmeh): rework
        }
        onSignUpFailed: showPopupError(error)
    }
}
