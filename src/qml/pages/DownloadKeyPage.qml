import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0
import MesResult 1.0

import "../theme"
import "../components"

Page {

    property string username

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    header: Header {
        title: qsTr("Download from the Cloud")
    }

    Form {
        id: form

        FormImage {
            source: "../resources/icons/Key.png"
        }

        FormSubtitle {
            text: qsTr("Your account information is securely stored in the cloud. Please enter your security word(s) to access it:")
        }

        FormInput {
            id: password
            label: qsTr("Password")
            placeholder: qsTr("Enter password")
            password: true
        }

        FormPrimaryButton {
            text: qsTr("Decrypt")
            onClicked: {
                if (password.text === '') {
                    window.showPopupError('Password can not be empty')
                }

                form.showLoading(qsTr("Downloading your private key..."))

                var future = Messenger.signInWithBackupKey(username, password.text)

                Future.onFinished(future, function(result) {
                    form.hideLoading()

                    if (Future.result(future) === Result.MRES_OK) {
                        mainView.showContacts(true)
                        return
                    }

                    window.showPopupError("Private key download error")
                })
            }
        }
    }

    footer: Footer {}
}

