import QtQuick 2.2
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.0

ApplicationWindow {
    id: applicationWindow
    visible: true
    visibility: "FullScreen"

    minimumWidth: 640
    minimumHeight: 480

    Row {
        id: labels
        Label { text: "Manufacture ID"; width: applicationWindow.width / 6 }
        Label { text: "Device type"; width: applicationWindow.width / 6  }
        Label { text: "MAC address"; width: applicationWindow.width / 6  }
        Label { text: "Device role"; width: applicationWindow.width / 6  }
        Label { text: "Firmware version"; width: applicationWindow.width / 6  }
        Label { text: "Trustlist version"; width: applicationWindow.width / 6  }
    }

    ListView {
        anchors.top: labels.bottom
        height: applicationWindow.height -labels.height

        flickableDirection: Flickable.AutoFlickDirection

        model: vsInfoGeneralController.vsInfoGeneralList
        delegate: Row {
            Label { text: manufacture_id; width: applicationWindow.width / 6  }
            Label { text: deviceType; width: applicationWindow.width / 6  }
            Label { text: default_netif_mac; width: applicationWindow.width / 6  }
            Label { text: device_roles; width: applicationWindow.width / 6  }
            Label { text: fw_ver; width: applicationWindow.width / 6  }
            Label { text: tl_ver; width: applicationWindow.width / 6  }
        }
    }
}
