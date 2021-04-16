import QtQuick 2.15

import "../components"

OperationPage {
    id: root

    appState: app.stateManager.downloadKeyState
    loadingText: qsTr("Downloading your private key...")

    property string username: ""

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
            onClicked: appState.downloadKey(root.username, password.text)
        }
    }
}

