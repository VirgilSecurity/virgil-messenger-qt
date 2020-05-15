import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

Page {

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: Header {
        title: "Settings"
    }

    Form {

        FormPrimaryButton {
            text: "Check updates"
            onClicked: {
                app.checkUpdates()
            }
        }

        FormPrimaryButton {
            text: "Delete account"
            onClicked: {
            }
        }

        FormPrimaryButton {
            text: "Send report"
            onClicked: {
                app.sendReport()
            }
        }

        FormPrimaryButton {
            text: "Sign out"
            onClicked: {
                mainView.signOut()
            }
        }
    }
}
