import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0
import MesResult 1.0

import "../theme"
import "../components"

Page {
    id: authenticationPage

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
            visible: !settings.usersList.length
            Layout.bottomMargin: 35
        }

        AccountSelection {
            visible: settings.usersList.length
            onUserSelected: {
                form.showLoading("Logging In as %1...".arg(userName))

                var future = Messenger.signIn(userName)
                Future.onFinished(future, (result) => {
                    var res = Future.result(future)
                    if (res === Result.MRES_OK) {
                        settings.lastSignedInUser = userName
                        showContacts(true)
                        return
                    }

                    if (res === Result.MRES_ERR_SIGNIN) {
                        root.showPopupError("Something went wrong")
                    }

                    form.hideLoading()
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



