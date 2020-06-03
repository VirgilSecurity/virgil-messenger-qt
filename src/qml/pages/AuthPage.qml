import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0
import MesResult 1.0

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
                form.showLoading("Logging In as %1...".arg(userName))

                var future = Messenger.signIn(userName)
                Future.onFinished(future, (result) => {
                    var res = Future.result(future)
                    if (res === Result.MRES_OK) {
                        form.hideLoading()
                        lastSignedInUser = userName
                        showContacts(true)
                    }
                })
            }
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
}



