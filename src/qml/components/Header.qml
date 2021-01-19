import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"

ToolBar {
    property alias title: titleLabel.text
    property bool showBackButton: true
    property alias showSeparator: separator.visible
    default property alias menu: contextMenu.contentData

    background: Rectangle {
        implicitHeight: Theme.headerHeight
        color: "transparent"

        HorizontalRule {
            id: separator
            anchors.leftMargin: Theme.margin
            anchors.rightMargin: Theme.margin
            anchors.bottom: parent.bottom
        }
    }

    RowLayout {
        anchors.fill: parent

        ImageButton {
            Layout.leftMargin: 12

            opacity: showBackButton ? 1 : 0
            enabled: showBackButton

            id: backButton
            image: "Arrow-Left"

            NetworkStatusControl {
                anchors {
                    centerIn: parent
                    verticalCenterOffset: 2
                    horizontalCenterOffset: 2
                }
                height: parent.iconSize
            }

            onClicked: app.stateManager.goBack()
        }

        Label {
            Layout.fillWidth: true

            id: titleLabel
            elide: Label.ElideRight

            font.family: Theme.mainFont
            font.pointSize: UiHelper.fixFontSz(15)
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            color: Theme.primaryTextColor
        }

        ImageButton {
            Layout.rightMargin: 12

            id: menuButton
            image: "More"
             // TODO(fpohtmeh): try with visibility
            opacity: contextMenu.count ? 1 : 0
            enabled: contextMenu.count
            onClicked: {
                contextMenu.open()
            }

            ContextMenu {
                id: contextMenu
            }
        }
    }
}
