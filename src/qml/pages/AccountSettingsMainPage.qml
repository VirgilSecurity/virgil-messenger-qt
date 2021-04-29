import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../theme"
import "../components"
import "../components/Dialogs"

Page {
    id: root

    signal editProfileRequested()
    signal backupKeyRequested()

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: Header {
        title: qsTr("Settings")
    }

    Form {
        Avatar {
            Layout.alignment: Qt.AlignHCenter
            diameter: Theme.bigAvatarDiameter
            nickname: controllers.users.currentUsername
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 50
            font.pointSize: UiHelper.fixFontSz(18)
            color: Theme.primaryTextColor
            text: controllers.users.currentUsername
        }

        FormLabel {
            text: qsTr("Version: %1").arg(app.currentVersion())
        }

        FormPrimaryButton {
            text: qsTr("Edit profile")
            onClicked: root.editProfileRequested()
            visible: false
        }

        FormPrimaryButton {
            text: qsTr("Backup private key")
            onClicked: root.backupKeyRequested()
        }

        FormPrimaryButton {
            text: qsTr("Send logs")
            onClicked: sendLogsDialog.open()
        }

        FormPrimaryButton {
            text: qsTr("Check updates")
            visible: Platform.isWindows
            onClicked: app.checkUpdates()
        }

        FormPrimaryButton {
            text: qsTr("Sign out")
            onClicked: messenger.signOut()
        }
    }

    InputDialog {
        id: sendLogsDialog
        title: qsTr("Send Logs")
        label: qsTr("Add details (optional)")
        onAccepted: crashReporter.sendLogFiles(text)
    }
}
