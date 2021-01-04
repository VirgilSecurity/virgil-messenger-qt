import QtQuick 2.12
import QtQuick.Controls 2.12

import "../theme"
import "../components"

Page {
    property var appState
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
                showPopupError(errorText) // TODO(fpohtmeh): don't use parent method directly
            }
        }
    }
}
