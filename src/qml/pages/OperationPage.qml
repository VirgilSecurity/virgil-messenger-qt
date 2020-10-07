import QtQuick 2.12
import QtQuick.Controls 2.12

import "../theme"
import "../components"

Page {
    property var state
    property bool loadingUsed: true
    property string loadingText: ""

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    footer: Footer {}

    Connections {
        target: state

        function onOperationStarted() {
            if (loadingUsed) {
                form.showLoading(loadingText)
            }
        }

        function onOperationFinished() {
            if (loadingUsed) {
                form.hideLoading()
            }
        }

        function onOperationErrorOccurred(errorText) {
            if (loadingUsed) {
                form.hideLoading()
                showPopupError(errorText) // TODO(fpohtmeh): don't use parent method directly
            }
        }
    }
}
