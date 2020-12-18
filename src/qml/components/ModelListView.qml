import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"
import "../components"
import "../components/CommonHelpers"

ListView {
    id: listView
    spacing: Theme.minSpacing
    clip: true

    signal placeholderClicked()

    property var searchHeader: undefined
    property bool isSearchOpened: searchHeader ? searchHeader.isSearchOpen : true
    property string search: searchHeader ? searchHeader.search : ""

    property alias emptyIcon: labeledIcon.emptyIcon
    property alias emptyText: labeledIcon.emptyListText
    readonly property bool isEmpty: listView.count === 0

    add: Theme.addTransitionNoScale
    displaced: Theme.displacedTransition
    move: Theme.moveTransition
    remove: Theme.removeTransition

    IconWithText {
        id: labeledIcon
        property url emptyIcon: "../resources/icons/Chats.png"
        property url searchIcon: "../resources/icons/Search_Big.png"
        property string emptyListText: qsTr("Create your first chat<br/>by pressing the dots<br/>button above")
        property string searchHintText: qsTr("Search results<br/>will appear here")
        property string searchNoResultsText: qsTr("Nothing found")
        visible: listView.isEmpty

        image {
            source: isSearchOpened ? searchIcon : emptyIcon
            width: 48
            height: 48
        }

        label {
            text: {
                if (!isSearchOpened) {
                    return emptyListText;
                }
                return search === '' ? searchHintText : searchNoResultsText
            }
            color: Theme.labelColor
        }
    }

    MouseArea {
        visible: listView.isEmpty
        anchors.fill: parent
        onClicked: {
            listView.placeholderClicked()
            mouse.accepted = false
        }
    }
}
