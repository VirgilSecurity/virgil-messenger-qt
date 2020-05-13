import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0

import "../theme"
import "../components"

Page {
    id: authenticationPage

    property var userList: Messenger.usersList()
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
            onUserSelected: {
                form.showLoading("Signing in as %1...".arg(userName))

                Future.onFinished(Messenger.signIn(userName), (result) => {
                    form.hideLoading()
                    lastSignedInUser = userName
                    showContacts()
                })
            }
        }

        FormPrimaryButton {
            text: "Register"
            onClicked: mainView.showRegister()
        }

        FormSecondaryButton {
            text: "Sign in"
            onClicked: mainView.showSignIn()
        }

    }

    footer: Footer {}
}



