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

    property bool isSelectable: true

    signal contactSelected(string contactUsername)

    QtObject {
        id: d

        readonly property var model: models.discoveredContacts
        property real headerOpacity: 0
        readonly property real defaultChatHeight: 50
        readonly property real selectionIconSize: 20
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
            height: d.defaultChatHeight
            hoverEnabled: true

            Avatar {
                id: avatar
                nickname: model.displayName
                avatarUrl: model.avatarUrl
                isSelected: model.isSelected
            }

            Column {
                Layout.fillWidth: true

                Text {
                    color: Theme.primaryTextColor
                    font.pointSize: UiHelper.fixFontSz(15)
                    text: model.displayName
                    elide: Text.ElideRight
                }

                Text {
                    color: Theme.secondaryTextColor
                    font.pointSize: UiHelper.fixFontSz(12)
                    text: model.details
                    width: parent.width
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                }
            }

            onClicked: {
                if (root.isSelectable) {
                    root.contactSelected(model.username)
                }
            }
        }
    }
}
