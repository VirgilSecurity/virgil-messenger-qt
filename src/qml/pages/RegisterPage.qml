import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

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
    }

    Form {

        FormInput {
            id: username
            objectName: "fiRegisterUsername"
            label: "Username"
        }

        FormPrimaryButton {
            objectName: "btnCreateAccount"
            onClicked: mainView.signUp(username.text)
            text: "Create account"
        }

        FormLabel {
            text: terms.arg(Theme.buttonPrimaryColor)
        }
    }

    footer: Footer {}
}
