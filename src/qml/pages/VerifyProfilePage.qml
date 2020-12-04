import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import com.virgilsecurity.messenger 1.0

import "../theme"
import "../components"

OperationPage {
    id: editProfilePage
    appState: app.stateManager.verifyProfileState

    loadingText: qsTr("Opening confirmation page...")

    header: Header {
        showBackButton: !confirmForm.isLoading
        title: qsTr("Confirm")
    }

    Form {
        id: confirmForm

        FormInput {
            id: confirmInput
            label: [qsTr("Confirm phone"), qsTr("Confirm email")][appState.codeType]
            password: false
            placeholder: qsTr("Enter code")
        }

        FormPrimaryButton {
            text: qsTr("Verify")
            enabled: confirmInput.text
            onClicked: appState.verify(confirmInput.text)
        }
    }

    Connections {
        target: appState

        function onVerificationFinished(codeType, success) {
            if (success) {
                root.showPopupSuccess([qsTr("Phone was confirmed"), qsTr("Email was confirmed")][codeType]);
                app.stateManager.goBack()
            } else {
                root.showPopupError(qsTr("Verification code isn't valid"))
            }
        }
    }
}
