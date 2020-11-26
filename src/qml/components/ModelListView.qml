import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"
import "../components"
import "../components/CommonHelpers"

ListView {
    id: listView

    signal placeholderClicked()

    property var searchHeader: undefined
    property alias emptyIcon: labeledIcon.emptyIcon
    property alias emptyText: labeledIcon.emptyText
    property int defaultAnimationDuration: 250
    property int shortAnimationDuration: 250

    cacheBuffer: 5000

    add: Transition {
        NumberAnimation {property: "scale"; from: 0.9; to: 1; duration: defaultAnimationDuration; easing.type: Easing.InOutCubic}
        NumberAnimation {property: "opacity"; from: 0; to: 1; duration: defaultAnimationDuration; easing.type: Easing.InOutCubic}
    }

    displaced: Transition {
        NumberAnimation {properties: "x,y"; duration: defaultAnimationDuration; easing.type: Easing.InOutCubic}
        NumberAnimation {property: "scale"; to: 1; duration: defaultAnimationDuration; easing.type: Easing.InOutCubic}
        NumberAnimation {property: "opacity"; to: 1; duration: defaultAnimationDuration; easing.type: Easing.InOutCubic}
    }

    move: Transition {
        NumberAnimation {properties: "x,y"; duration: defaultAnimationDuration; easing.type: Easing.InOutCubic}
    }

    remove: Transition {
        NumberAnimation {property: "opacity"; to: 0; duration: shortAnimationDuration; easing.type: Easing.InOutCubic}
        NumberAnimation {property: "scale"; to: 0.9; duration: shortAnimationDuration; easing.type: Easing.InOutCubic}
    }

    IconWithText {
        id: labeledIcon
        property url emptyIcon: "../resources/icons/Chats.png"
        property url searchIcon: "../resources/icons/Search_Big.png"
        property string emptyText: qsTr("Create your first chat<br/>by pressing the dots<br/>button above")
        property string searchText: qsTr("Search results<br/>will appear here")
        property string searchEmptyText: qsTr("Nothing found")
        visible: !listView.contentItem.children.length

        image {
            source: {
                if (searchHeader) {
                    if (searchHeader.isSearchOpen) {
                        return searchIcon
                    } else {
                        return emptyIcon
                    }
                }
            }

            width: 48
            height: 48
        }

        label {
            text: {
                if (searchHeader) {
                    if (!searchHeader.isSearchOpen) {
                        return emptyText;
                    }
                    if (searchHeader.search !== '') {
                        return searchEmptyText;
                    }
                    return searchText;
                }
            }

            color: Theme.labelColor
        }
    }

    MouseArea {
        visible: !listView.contentItem.children.length
        anchors.fill: parent
        onClicked: {
            listView.placeholderClicked()
            mouse.accepted = false
        }
    }
}
