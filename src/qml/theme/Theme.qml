pragma Singleton

import QtQuick 2.12
import QtQuick.Window 2.12

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
    readonly property color chatBackgroundColor: "#263238"
    readonly property color contactSelectionColor: "#2196F3"

    // sizes
    readonly property real margin: 20
    readonly property real spacing: 20
    readonly property real padding: 20
    readonly property real smallMargin: 10
    readonly property real smallSpacing: 10
    readonly property real smallPadding: 10
    readonly property real minSpacing: 3

    readonly property real avatarHeight: 40
    readonly property real avatarWidth: avatarHeight
    readonly property real avatarRadius: avatarWidth / 2

    readonly property real headerHeight: 60
    readonly property real formMaximumWidth: 260

    // icons
    readonly property url mainLogo: "../resources/icons/Logo-Big.png"

    // texts
    readonly property string mainLogoText: app.organizationDisplayName
    readonly property string mainFont: robotoRegular.name
    readonly property string mainFontBold: robotoBold.name

    // z indices
    readonly property real overlayZ: 1000

    // animation
    readonly property int animationDuration: 250
    readonly property int shortAnimationDuration: 125

    // transitions
    readonly property var addTransition: Transition {
        NumberAnimation { property: "scale"; from: 0.9; to: 1; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
    }
    readonly property var addTransitionNoScale: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
    }
    readonly property var moveTransition: Transition {
        NumberAnimation { properties: "x,y"; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
    }
    readonly property var displacedTransition: Transition {
        NumberAnimation { properties: "x,y"; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
        NumberAnimation { property: "scale"; to: 1; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
        NumberAnimation { property: "opacity"; to: 1; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
    }
    readonly property var removeTransition: Transition {
        NumberAnimation { property: "opacity"; to: 0; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
    }
}
