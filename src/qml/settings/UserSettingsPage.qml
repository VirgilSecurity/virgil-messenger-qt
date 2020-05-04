import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "../helpers/ui"
import "../theme"
import "../login/components"


Page {
    id: globalSettingsPage

    background: Rectangle {
        color: "transparent"
    }

    header: HeaderToolbar {
        title: qsTr("Settings")
    }
}
