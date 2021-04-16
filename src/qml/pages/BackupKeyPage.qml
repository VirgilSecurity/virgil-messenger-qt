import QtQuick 2.15

import "../components"

OperationPage {
    id: root

    appState: app.stateManager.backupKeyState
    loadingText: qsTr("Backing up your private key...")

    signal keyBackuped()

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
            onClicked: appState.backupKey(password.text, confirmPassword.text)
            text: qsTr("Backup")
        }
    }

    Connections {
        target: appState

        function onOperationFinished() {
            password.text = ""
            confirmPassword.text = ""
            notificationPopup.showSuccess(qsTr("Backup private key success"))
            root.keyBackuped()
        }
    }
}
