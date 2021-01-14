import QtQuick 2.12
import QtQuick.Controls 2.12

import "../base"
import "../theme"
import "../components"


Item {
    id: keyboardHandler

    property real keyboardHeight: 0
    property var constKeyboardRectangle
    property real constKeyboardYTranslate: 0
    property var currentKeyboardRectangle
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

    function onOpeningKeyboard() {
        if (isAnimationRequired()) {
            let keyboard = keyboardHandler.constKeyboardRectangle
            let keyboardYTranslate = keyboardHandler.constKeyboardYTranslate
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
        if (mainView.keyboardHeight === 0) {
            return
        }

        let currentKeyboard = keyboardHandler.currentKeyboardRectangle
        let constKeyboard = keyboardHandler.constKeyboardRectangle
        if (currentKeyboard.y !== constKeyboard.y) {
            keyboardAnimation.to = currentKeyboard.height - keyboardHandler.constKeyboardYTranslate
            keyboardAnimation.easing.type = Easing.InOutSine
            keyboardAnimation.duration = Theme.shortAnimationDuration
            keyboardAnimation.restart()
        }
    }

    function isAnimationRequired() {
        let globalPoint = mainView.mapFromItem(activeFocusItem, activeFocusItem.x, activeFocusItem.y)
        let activeItemBottomPoint = globalPoint.y + activeFocusItem.height

        let keyboard = keyboardHandler.constKeyboardRectangle
        let keyboardSpacing = Theme.margin * 2
        let keyboardTopPoint = mainView.height - keyboard.height - keyboardSpacing

        if (activeItemBottomPoint < keyboardTopPoint) {
            return false
        } else {
            return true
        }
    }

    function constKeyboardInit() {
        let keyboard = Qt.inputMethod.keyboardRectangle
        keyboardHandler.constKeyboardRectangle = Qt.rect(keyboard.x, keyboard.y, keyboard.width, keyboard.height)
        keyboardHandler.constKeyboardYTranslate = keyboard.y - mainView.height
    }

    Connections {
         target: visible ? Qt.inputMethod : null

         function onVisibleChanged() {
             if(visible && keyboardHandler.keyboardOpen) {
                 keyboardHandler.keyboardOpen = false;
             } else {
                 if (typeof keyboardHandler.constKeyboardRectangle === "undefined") {
                     keyboardHandler.constKeyboardInit()
                 }
                 keyboardHandler.keyboardOpen = true;
             }
         }

         function onKeyboardRectangleChanged() {
             keyboardHandler.currentKeyboardRectangle = Qt.inputMethod.keyboardRectangle
             keyboardHandler.onChangingKeyboard()
         }
     }

    NumberAnimation {
        id: keyboardAnimation
        target: keyboardHandler
        property: 'keyboardHeight'
    }
}
