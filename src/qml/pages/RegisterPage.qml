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
    }

    Form {

        FormInput {
            id: username
            label: "Username"
        }

        FormPrimaryButton {
            onClicked: signUp(username.text)
            text: "Create account"
        }

        FormLabel {
            text: terms.arg(Theme.buttonPrimaryColor)
        }
    }

    footer: Footer {}

    function signUp(user) {
        if (LoginLogic.validateUser(user)) {
            var future = Messenger.signUp(user)

            Future.onFinished(future, function(result) {

            })

            showPopupInform("Sign Up ...")
            lastSignedInUser = user
            showContacts()
        } else {
            showPopupError(qsTr("Incorrect user name"))
        }
    }
}
