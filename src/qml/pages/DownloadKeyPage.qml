import QtQuick 2.12

import "../components"

OperationPage {
    state: app.stateManager.downloadKeyState
    loadingText: qsTr("Downloading up your private key...")

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
            onClicked: app.stateManager.downloadKey(password.text)
        }
    }
}

