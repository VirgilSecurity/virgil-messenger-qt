import QtQuick 2.15
import QtQuick.Layouts 1.15

import "../components"

OperationPage {
    appState: app.stateManager.accountSelectionState

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
            onUserSelected: appState.requestSignIn(userName)
        }

        FormPrimaryButton {
            text: qsTr("Sign Up")
            onClicked: appState.requestSignUp()
        }

        FormSecondaryButton {
            text: qsTr("Sign In")
            onClicked: appState.requestSignInUsername()
        }
    }
}
