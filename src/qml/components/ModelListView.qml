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

    IconWithText {
        id: labeledIcon
        property url emptyIcon: "../resources/icons/Chats.png"
        property url searchIcon: "../resources/icons/Search_Big.png"
        property string emptyText: qsTr("Create your first chat<br/>by pressing the dots<br/>button above")
        property string searchText: qsTr("Search results<br/>will appear here")
        property string searchEmptyText: qsTr("Nothing found")
        visible: !listView.contentItem.children.length

        image {
            source: searchHeader.isSearchOpen ? searchIcon : emptyIcon
            width: 48
            height: 48
        }

        label {
            text: {
                if (!searchHeader.isSearchOpen) {
                    return emptyText;
                }
                if (searchHeader.search !== '') {
                    return searchEmptyText;
                }
                return searchText;
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
