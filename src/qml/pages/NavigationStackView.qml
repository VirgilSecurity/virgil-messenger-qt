import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

StackView {
    id: root
    property var appState: null

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    function navigatePush(component, params, transition) {
        root.push(component, params, transition)
    }

    function navigateReplace(component) {
        root.replace(null, component, StackView.Immediate)
    }

    function navigateBack(transition) {
        if (currentItem.navigateBack && currentItem.navigateBack(transition)) {
            return true
        } else if (depth > 1) {
            navigatePop(transition)
            return true
        } else {
            return false
        }
    }

    function navigatePop(transition) {
        root.pop(transition)
    }
}
