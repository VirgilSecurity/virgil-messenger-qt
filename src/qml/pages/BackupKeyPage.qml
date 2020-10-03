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

    footer: Footer { }

    Connections {
        target: app.stateManager.backupKeyState

        function onBackupKeyStarted() {
            form.showLoading(qsTr("Backing up your private key..."))
        }

        function onBackupKeyFinished() {
            form.hideLoading()
            showPopupSuccess(qsTr("Backup private key success"))
            password.text = ""
            confirmPassword.text = ""
        }

        function onBackupKeyErrorOccurred(errorText) {
            form.hideLoading()
            showPopupError(errorText) // TODO(fpohtmeh): don't use parent method directly
        }
    }
}

