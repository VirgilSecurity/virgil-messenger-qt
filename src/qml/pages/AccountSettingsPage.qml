import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

Page {

    property bool showServersPanel: true

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    header: Header {
        title: "Settings"
    }

    Form {

        FormPrimaryButton {
            text: "Software Update"
            onClicked: {
                app.checkUpdates()
            }
        }

        FormPrimaryButton {
            text: "Delete Account"
            onClicked: {
            }
        }

        FormPrimaryButton {
            text: "Send Report"
            onClicked: {
                app.sendReport()
            }
        }

        FormPrimaryButton {
            text: "Log Out"
            onClicked: {
                mainView.signOut()
            }
        }
    }
}
