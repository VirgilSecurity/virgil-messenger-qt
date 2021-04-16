import QtQuick 2.15
import QtQuick.Layouts 1.15

import "../components"

OperationPage {
    id: root

    signal signIn(string username)
    signal signInUsername()
    signal signUp()

    header: Header {
        showBackButton: false
        showSeparator: false
    }

    Form {
        id: form

        FormVendor {
            visible: settings.usersList.length === 0
            Layout.bottomMargin: 35 // TODO(fpohtmeh): move to constants
        }

        AccountSelection {
            visible: settings.usersList.length > 0
            onUserSelected: root.signIn(userName)
        }

        FormPrimaryButton {
            text: qsTr("Sign Up")
            onClicked: root.signUp()
        }

        FormSecondaryButton {
            text: qsTr("Sign In")
            onClicked: root.signInUsername()
        }
    }
}
