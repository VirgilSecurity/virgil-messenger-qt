import QtQuick 2.15
import QtQuick.Controls 2.15

import "../base"
import "../theme"
import "../components"


Item {
    id: keyboardHandler

    property real keyboardHeight: 0

    QtObject {
        id: kbd
        property var initKeyboardRectangle
        property real initKeyboardYTranslate: 0
        property bool keyboardOpen: false
        onKeyboardOpenChanged: {
            if (!Platform.isDesktop) {
                if (keyboardOpen) {
                    keyboardHandler.onOpeningKeyboard()
                } else {
                    keyboardHandler.onClosingKeyboard()
                }
            }
        }
    }

    function onOpeningKeyboard() {
        if (isAnimationRequired()) {
            let keyboard = kbd.initKeyboardRectangle
            let keyboardYTranslate = kbd.initKeyboardYTranslate
            let toPosition = keyboard.height - keyboardYTranslate
            keyboardAnimation.to = toPosition
            keyboardAnimation.easing.type = Easing.OutCubic
            keyboardAnimation.duration = Theme.animationDuration
            keyboardAnimation.restart()
        }
    }

    function onClosingKeyboard() {
        keyboardAnimation.to = 0
        keyboardAnimation.easing.type = Easing.OutCubic
        keyboardAnimation.duration = Theme.animationDuration
        keyboardAnimation.restart()
    }

    function onChangingKeyboard() {
        if (keyboardHandler.keyboardHeight === 0) {
            return
        }

        let currentKeyboard = Qt.inputMethod.keyboardRectangle
        let initKeyboard = kbd.initKeyboardRectangle
        if (currentKeyboard.y !== initKeyboard.y) {
            keyboardAnimation.to = currentKeyboard.height - kbd.initKeyboardYTranslate
            keyboardAnimation.easing.type = Easing.InOutSine
            keyboardAnimation.duration = Theme.shortAnimationDuration
            keyboardAnimation.restart()
        }
    }

    function isAnimationRequired() {
        let globalPoint = mainView.mapFromItem(activeFocusItem, activeFocusItem.x, activeFocusItem.y)
        let activeItemBottomPoint = globalPoint.y + activeFocusItem.height

        let keyboard = kbd.initKeyboardRectangle
        let keyboardSpacing = Theme.margin * 2
        let keyboardTopPoint = mainView.height - keyboard.height - keyboardSpacing

        if (activeItemBottomPoint < keyboardTopPoint) {
            return false
        } else {
            return true
        }
    }

    function keyboardInit() {
        let keyboard = Qt.inputMethod.keyboardRectangle
        kbd.initKeyboardRectangle = Qt.rect(keyboard.x, keyboard.y, keyboard.width, keyboard.height)
        kbd.initKeyboardYTranslate = keyboard.y - mainView.height
    }

    Connections {
         target: Qt.inputMethod
         enabled: visible

         function onVisibleChanged() {
             if(visible && kbd.keyboardOpen) {
                 kbd.keyboardOpen = false;
             } else {
                 if (typeof kbd.initKeyboardRectangle === "undefined") {
                     keyboardHandler.keyboardInit()
                 }
                 kbd.keyboardOpen = true;
             }
         }

         function onKeyboardRectangleChanged() {
             keyboardHandler.onChangingKeyboard()
         }
     }

    NumberAnimation {
        id: keyboardAnimation
        target: keyboardHandler
        property: 'keyboardHeight'
    }
}
