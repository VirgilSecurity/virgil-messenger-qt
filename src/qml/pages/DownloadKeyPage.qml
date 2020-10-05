import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0
import MesResult 1.0

import "../theme"
import "../components"

Page {
    readonly property variant state: app.stateManager.downloadKeyState

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
            onClicked: app.stateManager.downloadKey(state.contactId, password.text)
        }
    }

    Connections {
        target: state

        function onBackupKeyStarted() {
            form.showLoading(qsTr("Downloading up your private key..."))
        }

        function onBackupKeyFinished() {
            form.hideLoading()
        }

        function onBackupKeyErrorOccurred(errorText) {
            form.hideLoading()
            showPopupError(errorText) // TODO(fpohtmeh): don't use parent method directly
        }
    }

    footer: Footer {}
}

