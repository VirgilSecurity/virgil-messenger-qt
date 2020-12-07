import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

Page {
    property var appState: app.stateManager.accountSettingsState

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: Header {
        title: qsTr("Settings")
    }

    Form {
        Avatar {
            Layout.alignment: Qt.AlignHCenter
            diameter: 80
            nickname: appState.userId
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 50
            font.pointSize: UiHelper.fixFontSz(18)
            color: Theme.primaryTextColor
            text: appState.userId
        }

        FormLabel {
            text: qsTr("Version: %1").arg(app.currentVersion())
        }

        FormPrimaryButton {
            text: qsTr("Edit profile")
            onClicked: appState.editProfile()
            visible: true
        }

        FormPrimaryButton {
            text: qsTr("Backup private key")
            onClicked: appState.requestBackupKey(appState.userId)
        }

        FormPrimaryButton {
            text: qsTr("Sign out")
            onClicked: controllers.users.signOut()
        }
    }
}
