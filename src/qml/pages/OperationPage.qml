import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"
import "../components"

Page {
    property var appState: null
    property bool loadingUsed: true
    property string loadingText: ""
    property alias footerText: footerItem.text

    Binding { target: form; property: "loadingText"; value: loadingText }

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    footer: Footer {id: footerItem; text: qsTr("Powered by Virgil Security, Inc.")}

    Connections {
        target: appState

        function onOperationStarted() {
            if (loadingUsed) {
                form.isLoading = true
            }
        }

        function onOperationFinished() {
            if (loadingUsed) {
                form.isLoading = false
            }
        }

        function onOperationErrorOccurred(errorText) {
            if (loadingUsed) {
                form.isLoading = false
                notificationPopup.showError(errorText)
            }
        }
    }
}
