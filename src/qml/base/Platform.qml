pragma Singleton

import QtQuick 2.12

Item {
    property string name: ""

    readonly property bool isAndroid: name == "android"
    readonly property bool isIos: name == "ios"
    readonly property bool isLinux: name == "linux"
    readonly property bool isMacOs: name == "osx"
    readonly property bool isWindows: name == "windows"

    readonly property bool isDesktop: isLinux || isMacOs || isWindows
    readonly property bool isMobile: isAndroid || isIos

    function detect() {
        name = Qt.platform.os
        console.log("Platform name:", name)
    }
}
