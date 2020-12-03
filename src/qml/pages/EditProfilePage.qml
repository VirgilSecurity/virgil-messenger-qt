import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

OperationPage {
    id: editProfilePage
    appState: app.stateManager.editProfileState
    property bool isPhoneConfirmed: appState.isPhoneNumberConfirmed
    property bool isEmailConfirmed: appState.isEmailConfirmed
    property string whatToConfirm

    loadingText: qsTr("Opening profile page...")

    header: Header {
        showBackButton: !profileForm.isLoading
        title: qsTr("Profile")
    }

    Form {
        id: profileForm

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

        FormInput {
            id: phone
            label: isPhoneConfirmed ? qsTr("Confirmed phone") : qsTr("Phone")
            password: false
            placeholder: qsTr("Enter phone")
            text: appState.phoneNumber
            onTextChanged: appState.phoneNumber = text
            validator: RegExpValidator { regExp: /^[\+]?[(]?[0-9]{3}[)]?[-\s\.]?[0-9]{3}[-\s\.]?[0-9]{4,6}$/im }
            enabled: !isPhoneConfirmed
        }

        FormPrimaryButton {
            text: isPhoneConfirmed ? qsTr("Reset") : qsTr("Confirm")
            enabled: phone.acceptableInput
            onClicked: phoneButtonClicked(isPhoneConfirmed)
        }

        FormInput {
            id: email
            label: isEmailConfirmed ? qsTr("Confirmed email") : qsTr("Email")
            password: false
            placeholder: qsTr("Enter email")
            text: appState.email
            onTextChanged: appState.email = text
            validator: RegExpValidator { regExp:/\w+([-+.']\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*/ }
            enabled: !isEmailConfirmed
        }

        FormPrimaryButton {
            text: isEmailConfirmed ? qsTr("Reset") : qsTr("Confirm")
            enabled: email.acceptableInput
            onClicked: emailButtonClicked(isEmailConfirmed)
        }
    }

    Connections {
        target: appState

        function onVerificationFinished(result) {
            app.stateManager.goBack()

            if (result) {
                let msg = qsTr("You have succesfully confirmed your " + whatToConfirm)
                root.showPopupSuccess(msg)
            } else {
                let msg = qsTr("Oops, you entered the wrong verification code")
                root.showPopupError(msg)
            }

            console.log("[VERIFICATION] : ", result)
        }
    }

    function phoneButtonClicked(isPhoneConfirmed) {
        if (isPhoneConfirmed) {
            appState.resetPhone()
        } else {
            whatToConfirm = "phone"
            appState.verifyProfile(whatToConfirm)
        }
    }

    function emailButtonClicked(isEmailConfirmed) {
        if (isEmailConfirmed) {
            appState.resetEmail()
        } else {
            whatToConfirm = "email"
            appState.verifyProfile(whatToConfirm)
        }
    }
}
