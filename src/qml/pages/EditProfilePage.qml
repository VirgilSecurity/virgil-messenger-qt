import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../theme"
import "../components"

OperationPage {
    id: root
    appState: app.stateManager.editProfileState
    loadingText: qsTr("Opening profile page...")

    signal verificationRequested(int codeType)

    header: Header {
        showBackButton: !form.isLoading
        title: qsTr("Profile")
    }

    Form {
        id: form

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

        // PHONE

        FormInput {
            id: phone
            label: appState.isPhoneNumberConfirmed ? qsTr("Confirmed phone") : qsTr("Phone")
            password: false
            placeholder: qsTr("Enter phone")
            text: appState.phoneNumber
            onTextChanged: appState.phoneNumber = text
            validator: app.validator.rePhone
            inputHint: Qt.ImhDialableCharactersOnly
            enabled: !appState.isPhoneConfirmed
        }

        FormPrimaryButton {
            text: qsTr("Reset")
            visible: appState.isPhoneNumberConfirmed
            enabled: phone.acceptableInput
            onClicked: appState.phoneNumber = ""
        }

        FormPrimaryButton {
            text: qsTr("Confirm")
            visible: !appState.isPhoneNumberConfirmed
            enabled: phone.acceptableInput
            onClicked: root.verificationRequested(ConfirmationCodeTypes.phoneNumber)
        }

        // EMAIL

        FormInput {
            id: email
            label: appState.isEmailConfirmed ? qsTr("Confirmed email") : qsTr("Email")
            password: false
            placeholder: qsTr("Enter email")
            text: appState.email
            onTextChanged: appState.email = text
            validator: app.validator.reEmail
            inputHint: Qt.ImhEmailCharactersOnly
            enabled: !appState.isEmailConfirmed
        }

        FormPrimaryButton {
            text: qsTr("Reset")
            visible: appState.isEmailConfirmed
            enabled: email.acceptableInput
            onClicked: appState.email = ""
        }

        FormPrimaryButton {
            text: qsTr("Confirm")
            visible: !appState.isEmailConfirmed
            enabled: email.acceptableInput
            onClicked: root.verificationRequested(ConfirmationCodeTypes.email)
        }
    }
}
