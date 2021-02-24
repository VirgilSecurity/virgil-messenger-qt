import QtQuick 2.15
import QtQuick.Layouts 1.15

import "../theme"

ColumnLayout {
    id: root
    spacing: Theme.smallSpacing

    default property alias contents: stack.children

    Row {
        spacing: Theme.smallSpacing
        leftPadding: Theme.smallSpacing

        readonly property variant contents: stack.children

        Repeater {
            model: contents.length

            delegate: Item {
                width: label.width
                height: selection.y + selection.height

                readonly property var child: contents[index]
                readonly property bool isCurrent: index === stack.currentIndex

                Text {
                    id: label
                    color: Theme.primaryTextColor
                    verticalAlignment: Qt.AlignVCenter
                    text: child.tabTitle ? child.tabTitle : qsTr("Tab")
                    font.bold: parent.isCurrent
                }

                Rectangle {
                    id: selection
                    y: label.height + Theme.minSpacing
                    x: -Theme.minSpacing
                    height: 2
                    width: label.width + 2 * Theme.minSpacing
                    color: parent.isCurrent ? Theme.avatarBgColor : "transparent"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: stack.currentIndex = index
                }
            }
        }
    }

    StackLayout {
        id: stack
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: 0
    }
}
