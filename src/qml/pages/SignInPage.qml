import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

Page {
    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    header: Header {
        title: "Sign in"
    }

    Form {

        FormInput {
            id: username
            label: "Username"
            placeholder: "Enter username"
        }

        FormPrimaryButton {
            onClicked: {

                mainView.showSignInAs()
                // if (Messenger.usersList().searc)
                // mainView.signIn(username.text)
            }
            text: "Sign in"
        }

    }

    footer: Footer { }
}
