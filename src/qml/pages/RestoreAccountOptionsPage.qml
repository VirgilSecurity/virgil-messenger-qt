import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

Page {
    id: root

    property var appState: app.stateManager.signInAsState
    property string username: ""
    property string footerText: qsTr("Powered by Virgil Security, Inc.")

    signal downloadKeyRequested(string username)

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    header: Header {
        title: qsTr("Sign in as %1").arg(root.username)
    }

    Form {
        id: form

        FormImage {
            source: "../resources/icons/Lock.png"
        }

        FormSubtitle {
            text: qsTr("We couldn't find a key to this account on device. You can provide it with the options below:")
        }

        FormPrimaryButton {
            text: qsTr("Download from the Cloud")
            onClicked: root.downloadKeyRequested(root.username)
        }

        FormPrimaryButton {
            text: qsTr("Get from another device")
            enabled: false
        }

        FormSecondaryButton {
            text: qsTr("Use local file")
            enabled: false
        }
    }

    footer: Footer { text: footerText }
}
