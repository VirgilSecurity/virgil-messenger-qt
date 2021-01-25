import QtQuick 2.15
import QtQuick.Controls 2.15

import "../base"
import "../theme"
import "../components"


Item {
    id: keyboardHandler

    property real keyboardHeight: 0

    QtObject {
        id: keyboardProps
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
            let keyboard = keyboardProps.initKeyboardRectangle
            let keyboardYTranslate = keyboardProps.initKeyboardYTranslate
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
        let initKeyboard = keyboardProps.initKeyboardRectangle
        if (currentKeyboard.y !== initKeyboard.y) {
            keyboardAnimation.to = currentKeyboard.height - keyboardProps.initKeyboardYTranslate
            keyboardAnimation.easing.type = Easing.InOutSine
            keyboardAnimation.duration = Theme.shortAnimationDuration
            keyboardAnimation.restart()
        }
    }

    function isAnimationRequired() {
        let globalPoint = mainView.mapFromItem(activeFocusItem, activeFocusItem.x, activeFocusItem.y)
        let activeItemBottomPoint = globalPoint.y + activeFocusItem.height

        let keyboard = keyboardProps.initKeyboardRectangle
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
        keyboardProps.initKeyboardRectangle = Qt.rect(keyboard.x, keyboard.y, keyboard.width, keyboard.height)
        keyboardProps.initKeyboardYTranslate = keyboard.y - mainView.height
    }

    Connections {
         target: Qt.inputMethod
         enabled: visible

         function onVisibleChanged() {
             if(visible && keyboardProps.keyboardOpen) {
                 keyboardProps.keyboardOpen = false;
             } else {
                 if (typeof keyboardProps.initKeyboardRectangle === "undefined") {
                     keyboardHandler.keyboardInit()
                 }
                 keyboardProps.keyboardOpen = true;
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
