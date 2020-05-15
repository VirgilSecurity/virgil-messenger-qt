import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QuickFuture 1.0

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
                form.showLoading("Signing in as %1...".arg(mainView.lastSignedInUser))

                Future.onFinished(Messenger.signIn(mainView.lastSignedInUser), (result) => {
                    form.hideLoading()
                    showContacts(true)
                })

                return
            }

            showAuth()
        }
    }

    Form {
        id: form

        FormVendor {
            Layout.bottomMargin: 20
        }
    }

    footer: Footer {}
}
