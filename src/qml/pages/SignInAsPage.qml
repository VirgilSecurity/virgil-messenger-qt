import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

Page {

    property string username

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    header: Header {
        title: qsTr("Sign in as %1".arg(username))
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
            onClicked: {
                mainView.busy()
                mainView.showDownloadKey()
            }
        }

        FormPrimaryButton {
            text: qsTr("Get from another device")
        }

        FormSecondaryButton {
            text: qsTr("Use local file")
        }
    }

    footer: Footer {}
}
