import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

Page {
    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    header: Header {
        showBackButton: !form.isLoading
        title: qsTr("Sign in")
    }

    Timer {
        id: signInTimer
        interval: 1000; running: false; repeat: false;
        onTriggered: {            
            mainView.showSignInAs({ username: username.text })
            form.hideLoading()
        }
    }

    Form {

        id: form

        FormInput {
            id: username
            label: qsTr("Username")
            placeholder: qsTr("Enter username")
        }

        FormPrimaryButton {
            onClicked: {
                form.showLoading(qsTr("Signing in..."))
                signInTimer.start()

                // if (Messenger.usersList().searc)
                // mainView.signIn(username.text)
            }
            text: qsTr("Sign in")
        }

    }

    footer: Footer { }
}
