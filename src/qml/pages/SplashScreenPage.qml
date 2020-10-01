import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"
import "../components"

AuthorizationPage {
    authorizationState: app.stateManager.splashScreenState

    Form {
        id: form

        FormVendor {
            Layout.bottomMargin: 20
        }
    }
}
