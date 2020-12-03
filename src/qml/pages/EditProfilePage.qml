import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

//Q_PROPERTY(QString userId READ userId WRITE setUserId NOTIFY userIdChanged)
//Q_PROPERTY(QString phoneNumber READ phoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)
//Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
//Q_PROPERTY(bool isPhoneNumberConfirmed READ isPhoneNumberConfirmed WRITE setIsPhoneNumberConfirmed NOTIFY isPhoneNumberConfirmedChanged)
//Q_PROPERTY(bool isEmailConfirmed READ isEmailConfirmed WRITE setIsEmailConfirmed NOTIFY isEmailConfirmedChanged)
//Q_PROPERTY(QUrl avatarUrl READ avatarUrl WRITE setAvatarUrl NOTIFY avatarUrlChanged)

OperationPage {
    id: editProfilePage
    appState: app.stateManager.editProfileState
    property bool isPhoneConfirmed: appState.isPhoneNumberConfirmed
    property bool isEmailConfirmed: appState.isEmailConfirmed

    loadingText: qsTr("Opening profile page...")

    header: Header {
        showBackButton: !confirmForm.isLoading
        title: editProfilePage.state === "standard" ? qsTr("Profile") : qsTr("Confirm")
    }

    state: "standard"
    states: [
        State {
            name: "standard"
            PropertyChanges {
                target: profileForm
                opacity: 1
                enabled: true
                scale: 1
            }
            PropertyChanges {
                target: confirmForm
                opacity: 0
                enabled: false
                scale: 0.8
            }
        },
        State {
            name: "phone confirmation"
            PropertyChanges {
                target: profileForm
                opacity: 0
                enabled: false
                scale: 1.2
            }
            PropertyChanges {
                target: confirmForm
                opacity: 1
                enabled: true
                scale: 1
            }
        },
        State {
            name: "email confirmation"
            PropertyChanges {
                target: profileForm
                opacity: 0
                enabled: false
                scale: 1.2
            }
            PropertyChanges {
                target: confirmForm
                opacity: 1
                enabled: true
                scale: 1
            }
        }
    ]

    transitions: Transition {
        NumberAnimation {properties: "opacity, scale, enabled"; duration: Theme.animationDuration; easing.type: Easing.InExpo}
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

    Form {
        id: confirmForm

        FormInput {
            id: confirmInput
            label: {
                if (editProfilePage.state === "phone confirmation") {
                    return qsTr("Confirm phone")
                } else if (editProfilePage.state === "email confirmation") {
                    return qsTr("Confirm email")
                } else {
                    return ""
                }
            }

            password: false
            placeholder: qsTr("Enter code")
        }

        FormPrimaryButton {
            text: qsTr("Verify")
            onClicked: {
                console.log("[CONFIRMATION] : ", text)
                editProfilePage.state = "standard"
            }
        }

        FormSecondaryButton {
            text: qsTr("Cancel")
            onClicked: confirmationCanceled()
        }
    }

    function phoneButtonClicked(isPhoneConfirmed) {
        if (isPhoneConfirmed) {
            appState.resetPhone()

//            appState.phoneNumber = ""
//            appState.isPhoneNumberConfirmed = false
            // signal reset phone number
        } else {
            editProfilePage.state = "phone confirmation"
        }
    }

    function emailButtonClicked(isPhoneConfirmed) {
        if (isPhoneConfirmed) {
            appState.email = ""
            appState.isEmailConfirmed = false
            // signal reset email
        } else {
            editProfilePage.state = "email confirmation"
        }
    }

    function confirmationCanceled() {
        editProfilePage.state = "standard"
    }
}
