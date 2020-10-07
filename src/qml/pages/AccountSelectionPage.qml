import QtQuick 2.12
import QtQuick.Layouts 1.12

import "../components"

SignInPage {
    state: app.stateManager.accountSelectionState

    header: Header {
        showBackButton: false
        showSeparator: false
    }

    Form {
        id: form

        FormVendor {
            visible: !accountSelection.visible
            Layout.bottomMargin: 35
        }

        AccountSelection {
            id: accountSelection
            visible: settings.usersList.length
            onUserSelected: app.stateManager.signIn(userName)
        }

        FormPrimaryButton {
            text: qsTr("Sign Up")
            onClicked: app.stateManager.openSignUp()
        }

        FormSecondaryButton {
            text: qsTr("Sign In")
            onClicked: app.stateManager.openSignIn()
        }
    }
}
