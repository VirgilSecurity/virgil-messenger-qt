import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

ModelListView {
    id: root
    model: d.model.proxy
    delegate: contactListComponent
    section.delegate: contactSectionComponent
    section.property: "section"
    isSearchOpened: true

    property var selectionModel: d.model.selection
    property bool isSelectable: true
    property var itemContextMenu: null

    signal contactSelected(string contactUsername)

    QtObject {
        id: d
        readonly property var model: models.discoveredContacts
    }

    Component {
        id: contactSectionComponent

        Rectangle {
            width: root.width
            height: label.height + Theme.smallSpacing
            color: Theme.mainBackgroundColor

            Text {
                id: label
                x: Theme.smallSpacing
                y: 0.5 * Theme.smallSpacing
                width: parent.width - x
                color: Theme.secondaryTextColor
                text: section
                font.pointSize: UiHelper.fixFontSz(12)
            }
        }
    }

    Component {
        id: contactListComponent

        ListDelegate {
            id: delegate
            width: root.width
            height: Theme.headerHeight

            Avatar {
                id: avatar
                nickname: model.displayName
                avatarUrl: model.avatarUrl
                isSelected: model.isSelected
            }

            TwoLineLabel {
                Layout.fillWidth: true
                title: model.displayName
                description: model.details
            }

            ImageButton {
                id: menuButton
                image: "More"
                height: imageSize
                hoverVisible: !model.isSelected
                visible: root.isSelectable && root.itemContextMenu

                onClicked: {
                    root.selectionModel.clear()
                    root.selectionModel.toggle(model.index)
                    root.itemContextMenu.parent = menuButton
                    root.itemContextMenu.open()
                }
            }

            onSelectItem: {
                if (root.isSelectable) {
                    root.selectionModel.multiSelect = multiSelect
                    root.selectionModel.toggle(model.index)
                }
            }
        }
    }
}
