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
        readonly property bool debug: false

        function isLastComponent(component) {
            return components && components[0] === component
        }

        function enterState() { app.stateManager.enterState(currentItem.appState) }
    }

    function navigatePush(component, params, transition) {
        root.push(component, params, transition)
        if (d.debug) {
            console.log("NavigationStackView.navigatePush", root, currentItem)
        }
        d.components.push(component)
    }

    function navigateReplace(component) {
        if (!d.isLastComponent(component)) {
            root.replace(null, component, StackView.Immediate)
            if (d.debug) {
                console.log("NavigationStackView.navigateReplace", root, currentItem)
            }
            d.components = [component]
        }
    }

    function navigateBack(transition) {
        if (d.debug) {
            console.log("NavigationStackView.navigateBack#0", root, currentItem)
        }
        if (currentItem.navigateBack && currentItem.navigateBack(transition)) {
            return true
        } else if (depth > 1) {
            if (d.debug) {
                console.log("NavigationStackView.navigateBack#1", root, currentItem)
            }
            root.pop(transition)
            if (d.debug) {
                console.log("NavigationStackView.navigateBack#2", root, currentItem)
            }
            d.components.pop()
            return true
        } else {
            return false
        }
    }

    onCurrentItemChanged: d.enterState()
}
