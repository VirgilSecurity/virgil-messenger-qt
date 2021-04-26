import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

StackView {
    id: root
    property var appState: null

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    QtObject {
        id: d
        property var components: []
    }

    function navigatePush(component, params, transition) {
        root.push(component, params, transition)
        d.components.push(component)
    }

    function navigateReplace(component) {
        root.replace(null, component, StackView.Immediate)
        d.components = [component]
    }

    function navigateBack(transition) {
        if (currentItem.navigateBack && currentItem.navigateBack(transition)) {
            return true
        } else if (depth > 1) {
            root.pop(transition)
            d.components.pop()
            return true
        } else {
            return false
        }
    }
}
