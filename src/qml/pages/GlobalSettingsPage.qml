import QtQuick 2.12
import QtQuick.Controls 2.12

import "../components/Headers/"

Page {
    id: registerPage

    background: Rectangle {
        color: "transparent"
    }

    header: HeaderToolbar {
        title: qsTr("Settings")
    }
}
