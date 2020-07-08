import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0
import MesResult 1.0

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
            onClicked: {

                if (password.text === '') {
                    root.showPopupError('Password can not be empty')
                }

                if (password.text !== confirmPassword.text) {
                    root.showPopupError('Passwords are not match')
                }

                form.showLoading(qsTr("Backing up your private key..."))

                var future = Messenger.backupUserKey(password.text)

                Future.onFinished(future, function(result) {
                    form.hideLoading()

                    if (Future.result(future) === Result.MRES_OK) {
                        password.text = ''
                        confirmPassword.text = ''
                        root.showPopupSuccess('Backup private key success');
                        return
                    }

                    root.showPopupError("Backup private key error")
                })
            }
            text: qsTr("Backup")
        }

    }

    footer: Footer { }
}

