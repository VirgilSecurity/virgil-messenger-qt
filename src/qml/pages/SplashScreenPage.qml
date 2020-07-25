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
        interval: 1000;
        running: true;
        repeat: false;
        onTriggered: messenger.start()
    }

    Form {
        id: form

        FormVendor {
            Layout.bottomMargin: 20
        }
    }

    footer: Footer {}

    Connections {
        target: messenger
        onSignIn: form.showLoading(qsTr("Logging In as %1...").arg(userWithEnv))
        onSignedIn: {
            form.hideLoading()
            mainView.showContacts(true)
        }
        onSignInFailed: {
            form.hideLoading()
            window.showPopupError(qsTr("Sign-in failed. Please check username/password"))
        }
        // TODO(fpohtmeh): remove duplicated code in AuthPage
    }
}
