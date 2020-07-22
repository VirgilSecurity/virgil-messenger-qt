import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QuickFuture 1.0
// TODO(fpohtmeh): remove
//import MesResult 1.0

import "../theme"
import "../components"

Page {

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    Timer {
        interval: 1000; running: true; repeat: false;
        onTriggered: {
            if (settings.lastSignedInUser) {
                form.showLoading("Logging In as %1...".arg(settings.lastSignedInUser))

                var future = Messenger.signIn(settings.lastSignedInUser)
                Future.onFinished(future, (result) => {
                    var res = Future.result(future)
                    if (res === Result.MRES_OK) {
                        form.hideLoading()
                        showContacts(true)
                    } else {
                       settings.lastSignedInUser = ""
                       showAuth()
                    }
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
