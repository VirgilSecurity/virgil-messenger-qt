import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../theme"
import "../components"

OperationPage {
    id: root
    appState: app.stateManager.verifyProfileState
    loadingText: qsTr("Opening confirmation page...")

    property int codeType: ConfirmationCodeTypes.phoneNumber

    signal verified()

    header: Header {
        showBackButton: !form.isLoading
        title: qsTr("Confirm")
    }

    Form {
        id: form

        FormInput {
            id: confirmInput
            label: [qsTr("Confirm phone"), qsTr("Confirm email")][root.codeType]
            password: false
            placeholder: qsTr("Enter code")
        }

        FormPrimaryButton {
            text: qsTr("Verify")
            enabled: confirmInput.text
            onClicked: appState.verify(root.codeType, confirmInput.text)
        }
    }

    Connections {
        target: appState

        function onVerificationFinished(codeType, success) {
            if (success) {
                notificationPopup.showError([qsTr("Phone was confirmed"), qsTr("Email was confirmed")][root.codeType]);
                root.verified()
            } else {
                notificationPopup.showError(qsTr("Verification code isn't valid"))
            }
        }
    }
}
