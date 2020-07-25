import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0

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
            id: passwordInput
            label: qsTr("Password")
            password: true
            placeholder: qsTr("Enter password")
        }

        FormInput {
            id: confirmPasswordInput
            label: qsTr("Confirm password")
            password: true
            placeholder: qsTr("Enter password again")
        }

        FormPrimaryButton {
            onClicked: {
                if (passwordInput.text === "")
                    window.showPopupError("Password can not be empty")
                else if (passwordInput.text !== confirmPasswordInput.text)
                    window.showPopupError("Passwords don't not match")
                else
                    messenger.backupKey(passwordInput.text)
            }
            text: qsTr("Backup")
        }
    }

    footer: Footer {}

    Connections {
        target: messenger
        onBackupKey: {
            form.showLoading(qsTr("Backing up your private key..."))
        }
        onKeyBackuped: {
            form.hideLoading();
            passwordInput.text = ""
            confirmPasswordInput.text = ""
            window.showPopupSuccess("Backup private key success")
            mainView.back() // TODO(fpohtmeh): remove main view
        }
        onBackupKeyFailed: {
            form.hideLoading()
            window.showPopupError(error)
            mainView.back() // TODO(fpohtmeh): remove main view
        }
    }
}

