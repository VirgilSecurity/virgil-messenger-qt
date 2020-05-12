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
            text: qsTr("You have backed up this account's key to Virgil Cloud. Enter the password to decrypt it:")
        }

        FormInput {
            label: qsTr("Password")
            placeholder: qsTr("Enter password")
        }

        FormPrimaryButton {
            text: qsTr("Decrypt")
        }
    }

    footer: Footer {}
}
