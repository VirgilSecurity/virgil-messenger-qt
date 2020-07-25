import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

Page {
    id: authenticationPage

    property var userList: settings.usersList
    property var loginPage
    property var registerPage

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    header: Header {
        showBackButton: false
        showSeporator: false
    }

    Form {
        id: form

        FormVendor {
            visible: !userList.length
            Layout.bottomMargin: 35
        }

        AccountSelection {
            visible: userList.length
            onUserSelected: messenger.signIn(userName)
        }

        FormPrimaryButton {
            text: "Sign Up"
            objectName: "btnRegister"
            onClicked: mainView.showRegister()
        }

        FormSecondaryButton {
            text: "Log In"
            onClicked: mainView.showSignIn()
        }

    }

    footer: Footer {}

    Connections {
        target: messenger
        onSignIn: form.showLoading(qsTr("Logging In as %1...").arg(userWithEnv))
        onSignedIn: {
            form.hideLoading()
            mainView.showContacts(true)
        }
        onSignInFailed: {
            form.hideLoading()
            window.showPopupError(qsTr("Sign-in failed. Please check username/password"))
        }
    }
}



