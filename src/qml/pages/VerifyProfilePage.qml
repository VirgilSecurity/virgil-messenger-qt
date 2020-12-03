import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

OperationPage {
    id: editProfilePage
    appState: app.stateManager.verifyProfileState
    property string whatToConfirm: appState.whatToConfirm

    loadingText: qsTr("Opening confirmation page...")

    header: Header {
        showBackButton: !confirmForm.isLoading
        title: qsTr("Confirm")
    }

    Form {
        id: confirmForm

        FormInput {
            id: confirmInput
            label: {
                if (whatToConfirm === "phone") {
                    return qsTr("Confirm phone")
                } else if (whatToConfirm === "email") {
                    return qsTr("Confirm email")
                } else {
                    return ""
                }
            }

            password: false
            placeholder: qsTr("Enter code")
        }

        FormPrimaryButton {
            text: qsTr("Verify")
            enabled: confirmInput.text
            onClicked: {
                appState.verifyProfileData(confirmInput.text)
            }
        }
    }
}
