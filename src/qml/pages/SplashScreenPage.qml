import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QuickFuture 1.0
import MesResult 1.0

import "../base"
import "../theme"
import "../components"

Page {
    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    Timer {
        interval: Platform.isAndroid ? 100 : 1000;
        running: true;
        repeat: false;
        onTriggered: Messenger.signIn(settings.lastSignedInUser)
    }

    Form {
        id: form

        FormVendor {
            Layout.bottomMargin: 20
        }
    }

    footer: Footer {}

    Connections {
        target: Messenger

        function onSignInStarted(userId) {
            Messenger.hideSplashScreen()
            form.showLoading("Logging in as %1...".arg(userId))
        }

        function onSignInUserEmpty() {
            Messenger.hideSplashScreen()
            showAuth()
        }

        function onSignedIn(userId) {
            form.hideLoading()
            showContacts(true)
        }

        function onSignInErrorOccured(errorText) {
            showAuth()
        }
    }
}
