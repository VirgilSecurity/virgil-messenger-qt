import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

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

        FormVendor {
            visible: !userList.length
            Layout.bottomMargin: 35
        }

        AccountSelection {
            visible: userList.length
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



