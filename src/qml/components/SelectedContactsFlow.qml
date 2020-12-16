import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

FlowListView {
    id: flowListView
    clip: true
//    spacing: d.flowSpacing
    model: selectedModel.proxy

    readonly property var selectedModel: models.discoveredContacts.selectedContacts
    readonly property var editedModel: models.discoveredContacts
    readonly property real recommendedHeight: d.recommendedHeight

    QtObject {
        id: d

        readonly property real flowItemHeight: 33
        readonly property real flowSpacing: 3
        readonly property real lineWidth: 2
        readonly property real recommendedHeight: Math.min(flowListView.contentHeight, 3 * (d.flowItemHeight + root.spacing) - root.spacing)
    }

    delegate: Item {
        height: d.flowItemHeight
        width: row.width + row.spacing

        Rectangle {
            id: flowItemRec
            anchors {
                fill: parent
                margins: d.flowSpacing
            }

            color: isSelected ? Theme.buttonPrimaryColor : Theme.contactPressedColor
            radius: height

            MouseArea {
                anchors.fill: parent
                onClicked: selectedModel.selection.toggle(index)
            }

            Row {
                id: row
                spacing: Theme.smallSpacing
                anchors.verticalCenter: parent.verticalCenter
                Item {
                    height: flowItemRec.height
                    width: height
                    Item {
                        anchors.fill: parent
                        visible: isSelected
                        Repeater {
                            model: 2
                            Rectangle {
                                anchors.centerIn: parent
                                width: 0.5 * parent.width
                                height: d.lineWidth
                                radius: height
                                color: Theme.brandColor
                                rotation: index ? -45 : 45
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: editedModel.toggleById(model.contactId)
                        }
                    }

                    Avatar {
                        id: avatar
                        nickname: model.name
                        avatarUrl: model.avatarUrl
                        diameter: d.flowItemHeight - (d.flowSpacing * 2)
                        anchors.verticalCenter: parent.verticalCenter
                        visible: !isSelected
                    }
                }

                Text {
                    color: Theme.primaryTextColor
                    font.pointSize: UiHelper.fixFontSz(15)
                    text: model.name
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
