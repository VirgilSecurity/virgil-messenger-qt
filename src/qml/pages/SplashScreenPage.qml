import QtQuick 2.12
import QtQuick.Layouts 1.12

import "../components"

SignInPage {
    state: app.stateManager.splashScreenState

    Form {
        id: form

        FormVendor {
            Layout.bottomMargin: 20
        }
    }
}
