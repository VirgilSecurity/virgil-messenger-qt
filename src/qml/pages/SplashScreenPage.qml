import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"
import "../components"

Page {
    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    Form {
        id: form

        FormVendor {
            Layout.bottomMargin: 20
        }
    }

    footer: Footer {}

    Connections {
        target: app.stateManager.splashScreenState

        function onSignInStarted(userId) {
            form.showLoading("Sign in as %1...".arg(userId))
        }

        function onSignInFinished() {
            form.hideLoading()
        }

        function onSignInErrorOccurred(errorText) {
            form.hideLoading()
            showPopupError(errorText) // TODO(fpohtmeh): don't use parent method directly
        }
    }
}
