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
        title: qsTr("Log In")
    }

    Form {

        id: form

        UserNameFormInput {
            id: username
            placeholder: qsTr("Enter Username")
        }

        FormPrimaryButton {
            onClicked: {

                if (username.text === '') {
                    window.showPopupError('Username is empty')
                }

                mainView.showSignInAs({ username: username.text })
            }
            text: qsTr("Log In")
        }

    }

    footer: Footer { }
}
