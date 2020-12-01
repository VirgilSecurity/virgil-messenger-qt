import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

OperationPage {
    appState: app.stateManager.editProfileState
    property string phoneValue: "+3809-test-test"
    property bool phoneConfirmed: false

    property string emailValue: "email@test.com"
    property bool emailConfirmed: true

//    appState: app.stateManager.accountSettingsState

    loadingText: qsTr("Opening profile page...")

    header: Header {
        showBackButton: !form.isLoading
        title: qsTr("Profile")
    }

    Form {
        id: form

        Avatar {
            Layout.alignment: Qt.AlignHCenter
            diameter: 80
//            nickname: appState.userId
            nickname: "ER"
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 50
            font.pointSize: UiHelper.fixFontSz(18)
            color: Theme.primaryTextColor
//            text: appState.userId
            text: "ERROR_404"
        }

        FormInput {
            id: phone
            label: qsTr("Phone")
            password: false
            placeholder: qsTr("Enter phone")
        }

        FormInput {
            id: email
            label: qsTr("Email")
            password: false
            placeholder: qsTr("Enter email")
            text: emailValue + " ✔" // Would be great to add ✔ icon if this item is confirmed
        }

        FormPrimaryButton {
//            onClicked: appState.backupKey(password.text, confirmPassword.text)
            text: qsTr("Confirm")
        }
    }

    Connections {
        target: appState

        function onOperationFinished() {
            showPopupSuccess(qsTr("Backup private key success"))
            password.text = ""
            confirmPassword.text = ""
        }
    }
}
