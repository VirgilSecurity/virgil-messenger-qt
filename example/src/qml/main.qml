import QtQuick 2.2
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.0

ApplicationWindow {
    id: applicationWindow
    visible: true
    visibility: VSQDeviceInfoController.visibilityMode

    minimumWidth: 640
    minimumHeight: 480

    Row {
        id: labels
        Label { text: "MAC address / Roles"; width: applicationWindow.width / 6 }
        Label { text: "Device state"; width: applicationWindow.width / 6 }
        Label { text: "Manufacture ID / Device Type"; width: applicationWindow.width / 6 }
        Label { text: "Firmware / Trustlist version"; width: applicationWindow.width / 6 }
        Label { text: "Sent / Received"; width: applicationWindow.width / 6 }
        Label { text: "Last timestamp"; width: applicationWindow.width / 6 }
    }

    ListView {
        anchors.top: labels.bottom
        height: applicationWindow.height -labels.height

        flickableDirection: Flickable.AutoFlickDirection

        model: VSQDeviceInfoController.deviceInfoList
        delegate: Row {
            Label { text: macDeviceRoles; width: applicationWindow.width / 6 }
            Label { text: deviceState; width: applicationWindow.width / 6 }
            Label { text: manufactureIdDeviceType; width: applicationWindow.width / 6 }
            Label { text: fwTlVer; width: applicationWindow.width / 6 }
            Label { text: statistics; width: applicationWindow.width / 6 }
            Label { text: lastTimestamp; width: applicationWindow.width / 6 }
        }
    }
}
