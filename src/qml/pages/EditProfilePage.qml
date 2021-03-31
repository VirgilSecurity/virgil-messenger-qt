import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

OperationPage {
    id: editProfilePage
    appState: app.stateManager.editProfileState

    loadingText: qsTr("Opening profile page...")

    header: Header {
        showBackButton: !profileForm.isLoading
        title: qsTr("Profile")
    }

    Form {
        id: profileForm

        Avatar {
            Layout.alignment: Qt.AlignHCenter
            diameter: 80 // TODO(fpohtmeh): move to theme
            nickname: controllers.users.currentUsername
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 50
            font.pointSize: UiHelper.fixFontSz(18)
            color: Theme.primaryTextColor
            text: controllers.users.currentUsername
        }

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
            text: appState.isPhoneNumberConfirmed ? qsTr("Reset") : qsTr("Confirm")
            enabled: phone.acceptableInput
            onClicked: phoneButtonClicked()
        }

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
            text: appState.isEmailConfirmed ? qsTr("Reset") : qsTr("Confirm")
            enabled: email.acceptableInput
            onClicked: emailButtonClicked()
        }
    }

    function phoneButtonClicked() {
        if (appState.isPhoneNumberConfirmed) {
            appState.phoneNumber = ""
        } else {
            appState.verify(Enums.ConfirmationCodeType.Phone)
        }
    }

    function emailButtonClicked() {
        if (appState.isEmailConfirmed) {
            appState.email = ""
        } else {
            appState.verify(Enums.ConfirmationCodeType.Email)
        }
    }
}
