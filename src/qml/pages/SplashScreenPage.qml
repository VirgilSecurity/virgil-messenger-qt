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
        onTriggered: {
            if (!settings.lastSignedInUser) {
                showAuth()
                Messenger.hideSplashScreen()
            }
            else {
                form.showLoading("Logging In as %1...".arg(settings.lastSignedInUser))

                var future = Messenger.signIn(settings.lastSignedInUser)
                Future.onFinished(future, function(res) {
                    if (res == Result.MRES_OK) {
                        form.hideLoading()
                        showContacts(true)
                    } else {
                        settings.lastSignedInUser = ""
                        showAuth()
                    }
                    Messenger.hideSplashScreen()
                })
            }
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
