import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

FlowListView {
    id: addedContactsView
    clip: true
    spacing: flowSpacing
    focus: true

    readonly property int flowItemHeight: 30
    readonly property int flowSpacing: 3
    readonly property int lineWidth: 2

    delegate: Rectangle {
        id: contactRec
        height: flowItemHeight
        width: row.width + row.spacing
        color: addedContactsView.currentIndex === index ? Theme.buttonPrimaryColor : Theme.contactPressedColor
        radius: height

        MouseArea {
            anchors.fill: parent
            onClicked: {
                addedContactsView.focus = true
                if (addedContactsView.currentIndex !== index) {
                    addedContactsView.currentIndex = index
                } else {
                    addedContactsView.currentIndex = -1
                }
            }
        }

        Row {
            id: row
            spacing: Theme.smallSpacing
            anchors.verticalCenter: parent.verticalCenter
            Item {
                height: flowItemHeight
                width: height
                Item {
                    height: flowItemHeight
                    width: height
                    visible: addedContactsView.currentIndex === index
                    enabled: addedContactsView.currentIndex === index
                    Repeater {
                        model: 2
                        Rectangle {
                            anchors.centerIn: parent
                            width: 0.5 * parent.width
                            height: lineWidth
                            radius: height
                            color: Theme.brandColor
                            rotation: index ? -45 : 45
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            addedContactsView.currentIndex = -1
                            addedContactsView.model.remove(index)
                        }
                    }
                }

                Avatar {
                    id: avatar
                    nickname: model.name
                    avatarUrl: model.avatarUrl
                    diameter: flowItemHeight
                    anchors.verticalCenter: parent.verticalCenter
                    visible: addedContactsView.currentIndex !== index
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

    Behavior on height {
        NumberAnimation {
            easing.type: Easing.InOutCubic
            duration: Theme.animationDuration
        }
    }

    onFocusChanged: {
        if (!focus) {
            addedContactsView.currentIndex = -1
        }
    }
}
