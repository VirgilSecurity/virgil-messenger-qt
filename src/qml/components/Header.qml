import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"

ToolBar {

    property alias title: titleLabel.text
    property bool showBackButton: true
    property alias showSeporator: seporator.visible
    default property alias menu: contextMenu.contentData

    background: Rectangle {
        implicitHeight: 60
        color: "transparent"

        Rectangle {
            id: seporator
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 20
            anchors.rightMargin: 20

            height: 1
            color: Theme.chatBackgroundColor
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

            // TODO: Componets sholdn't know about mainView
            // The logic must be as abastract as possible.
            onClicked: mainView.back()
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
            opacity: contextMenu.length ? 1 : 0
            enabled: contextMenu.length
            onClicked: {
                contextMenu.open()
            }

            ContextMenu {
                id: contextMenu
            }
        }
    }
}
