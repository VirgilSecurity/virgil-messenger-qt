import QtQuick 2.12

import "../components"

OperationPage {
    state: app.stateManager.backupKeyState
    loadingText: qsTr("Backing up your private key...")

    header: Header {
        showBackButton: !form.isLoading
        title: qsTr("Backup private key")
    }

    Form {
        id: form

        FormInput {
            id: password
            label: qsTr("Password")
            password: true
            placeholder: qsTr("Enter password")
        }

        FormInput {
            id: confirmPassword
            label: qsTr("Confirm password")
            password: true
            placeholder: qsTr("Enter password again")
        }

        FormPrimaryButton {
            onClicked: app.stateManager.backupKey(password.text, confirmPassword.text)
            text: qsTr("Backup")
        }
    }

    Connections {
        target: state

        function onOperationFinished() {
            showPopupSuccess(qsTr("Backup private key success"))
            password.text = ""
            confirmPassword.text = ""
        }
    }
}
