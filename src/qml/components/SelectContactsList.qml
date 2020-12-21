import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

ModelListView {
    id: modelListView
    model: d.model.proxy
    delegate: contactListComponent
    section.delegate: contactSectionComponent
    section.property: "section"

    signal contactSelected(string contactId)

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
            width: modelListView.width
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
            id: contactListDelegate
            width: modelListView.width
            height: d.defaultChatHeight

            Row {
                Layout.preferredHeight: parent.height
                spacing: Theme.smallSpacing

                add: Theme.addTransition
                move: Theme.moveTransition

                Image {
                    width: d.selectionIconSize
                    height: width
                    source: "../resources/icons/Check.png"
                    fillMode: Image.PreserveAspectFit
                    visible: isSelected ? true : false
                    anchors.verticalCenter: parent.verticalCenter
                }

                Avatar {
                    id: avatar
                    nickname: model.name
                    avatarUrl: model.avatarUrl
                    anchors.verticalCenter: parent.verticalCenter
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(15)
                        text: model.name
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
            }

            onClicked: {
                if (d.model.selection.multiSelect) {
                    d.model.toggleById(model.contactId)
                }
                contactSelected(contactId)
            }
        }
    }
}
