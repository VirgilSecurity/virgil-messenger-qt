import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"
import "../components"

Page {

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    Timer {
        interval: 1000; running: true; repeat: false;
        onTriggered: {
            if (mainView.lastSignedInUser) {
                 signIn(lastSignedInUser)
                return
            }

            showAuth()
        }
    }

    Form {
        FormVendor {
            Layout.bottomMargin: 20
        }

        FormLabel {
            id: statusLabel
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Component.onCompleted: {
        statusLabel.text = "Loading user data..."
    }

    footer: Footer {}
}
