import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

Page {
    property var authorizationState

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    footer: Footer {}

    Connections {
        target: authorizationState

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

        function onSignUpStarted(userId) {
            form.showLoading("Registering %1...".arg(userId))
        }

        function onSignUpFinished() {
            form.hideLoading()
        }

        function onSignUpErrorOccurred(errorText) {
            form.hideLoading()
            showPopupError(errorText) // TODO(fpohtmeh): don't use parent method directly
        }
    }
}
