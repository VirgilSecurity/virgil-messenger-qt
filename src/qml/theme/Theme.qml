pragma Singleton

import QtQuick 2.0
import QtQuick.Window 2.2

Item {
    FontLoader {
        id: robotoRegular
        source: "../resources/fonts/Roboto-Medium.ttf"
    }

    FontLoader {
        id: robotoBold
        source: "../resources/fonts/Roboto-Bold.ttf"
    }

    // colors
    readonly property color mainBackgroundColor : "#13191C"
    readonly property color contactsBackgroundColor : "#202A2F"
    readonly property color color : "#FF0000"
    readonly property color buttonPrimaryColor: "#D32F2F"
    readonly property color brandColor: "white"
    readonly property color primaryTextColor: "#FFFFFF"
    readonly property color secondaryTextColor: "#B0BEC5"
    readonly property color labelColor: "#6F828C"
    readonly property color chatSeparatorColor: "#314048"
    readonly property color menuBackgroundColor: "#7D929E"
    readonly property color avatarBgColor: "#1976D2"
    readonly property color avatarTextColor: "#80FFFFFF"
    readonly property color messegeCountColor: "#F44336"
    readonly property color contactPressedColor: "#455A64"
    readonly property color menuSeparatorColor: "#90A4AE"
    readonly property color inputBackgroundColor: "#37474F"
    readonly property color toolbarSepratatorColor: "#263238"


    // sizes
    readonly property real avatarHeight: 40
    readonly property real avatarWidth: avatarHeight
    readonly property real avatarRadius: avatarWidth / 2

    // icons
    readonly property url mainLogo: "../resources/icons/Logo-Big.png"

    // texts
    readonly property string mainLogoText: "Virgil"
    readonly property string mainFont: robotoRegular.name
    readonly property string mainFontBold: robotoBold.name
}




