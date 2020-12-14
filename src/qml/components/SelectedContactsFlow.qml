import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

FlowListView {
    id: root
    clip: true
    spacing: flowSpacing
    focus: true

    readonly property real recommendedHeight: flowItemHeight

    readonly property real flowItemHeight: 30
    readonly property real flowSpacing: 3
    readonly property real lineWidth: 2

    delegate: Rectangle {
        height: flowItemHeight
        width: row.width + row.spacing
        color: root.currentIndex === index ? Theme.buttonPrimaryColor : Theme.contactPressedColor
        radius: height

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.focus = true
                if (root.currentIndex !== index) {
                    root.currentIndex = index
                } else {
                    root.currentIndex = -1
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
                    visible: root.currentIndex === index
                    enabled: root.currentIndex === index
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
                            root.currentIndex = -1
                            root.model.remove(index)
                        }
                    }
                }

                Avatar {
                    id: avatar
                    nickname: model.name
                    avatarUrl: model.avatarUrl
                    diameter: flowItemHeight
                    anchors.verticalCenter: parent.verticalCenter
                    visible: root.currentIndex !== index
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
            root.currentIndex = -1
        }
    }
}
