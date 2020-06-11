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
        title: qsTr("Download from the Cloud")
    }

    Form {

        FormImage {
            source: "../resources/icons/Key.png"
        }

        FormSubtitle {
            text: qsTr("Your account information is securely stored in the cloud. Please enter your security word(s) to access it:")
        }

        FormInput {
            label: qsTr("Password")
            placeholder: qsTr("Enter Password")
        }

        FormPrimaryButton {
            text: qsTr("Decrypt")
        }
    }

    footer: Footer {}
}
