pragma Singleton

import QtQuick 2.0

QtObject {
    // colors
    readonly property color mainBackgroundColor : "#000000"
    readonly property color contactsBackgroundColor : "#202A2F"
    readonly property color color : "#FF0000"
    readonly property color buttonPrimaryColor: "#37474F"
    readonly property color primaryTextColor: "#FFFFFF"
    readonly property color brandColor: "#D32F2F"
    readonly property color secondaryFontColor: "#B0BEC5"
    readonly property color labelColor: "#6F828C"
    readonly property color sepratorColor: "#314048"
    readonly property color menuBackgroundColor: "#7D929E"
    readonly property color avatarBgColor: "#1976D2"
    readonly property color avatarTextColor: "#80FFFFFF"
    readonly property color messegeCountColor: "#F44336"

    // sizes
    readonly property real avatarHeight: 40
    readonly property real avatarWidth: avatarHeight
    readonly property real avatarRadius: avatarWidth / 2

    // icons
    readonly property url mainLogo: "../resources/icons/Logo-Big.svg"

    // texts
    readonly property string mainLogoText: "virgil"
    readonly property string mainFont: "Roboto"
}




