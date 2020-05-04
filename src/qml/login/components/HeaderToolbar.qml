import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "../../helpers/ui"
import "../../theme"

ToolBar {
    property string title

    background: Rectangle {
        implicitHeight: 60
        color: "transparent"

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            height: 1
            color: Theme.chatBackgroundColor
            anchors.bottom: parent.bottom
        }
    }

    width: parent.width

    RowLayout {
        id: layout
        anchors.fill: parent

        ToolButton {
            background: Rectangle {
                color: "transparent"
                implicitHeight: 60
                implicitWidth: 60
            }
            icon.source: "../../resources/icons/Arrow-Left.png"
            icon.height: 24
            icon.width: 24
            icon.color: Theme.secondaryTextColor
            onClicked: mainView.back()
            font.pointSize: UiHelper.fixFontSz(24)
        }

        Label {
            text: title
            font.pointSize: UiHelper.fixFontSz(15)
            color: Theme.primaryTextColor
            elide: Label.ElideRight
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: true
        }

        Rectangle {
            color: "transparent"
            width: 60
            height: 60
        }
    }
}
