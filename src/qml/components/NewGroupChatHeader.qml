import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"

ToolBar {
    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property bool showBackButton: true
    property alias showSeparator: separator.visible
    property alias rightButtonEnabled: rightButton.enabled

    signal actionButtonClicked()

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

            onClicked: app.stateManager.goBack()
        }

        Column {
            id: titleDescriptionColumn
            Layout.fillWidth: true

            Label {
                id: titleLabel
                elide: Label.ElideRight
                anchors.horizontalCenter: parent.horizontalCenter

                font.family: Theme.mainFont
                font.pointSize: UiHelper.fixFontSz(15)
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                font.bold: true
                color: Theme.primaryTextColor
            }

            Label {
                id: descriptionLabel
                elide: Label.ElideRight
                anchors.horizontalCenter: parent.horizontalCenter

                font.family: Theme.mainFont
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                font.pointSize: UiHelper.fixFontSz(11)
                color: Theme.secondaryTextColor
            }

        }

        ImageButton {
            Layout.rightMargin: 12

            id: rightButton
            image: "Send"
            opacity: enabled ? 1 : 0.5

            onClicked: actionButtonClicked()
        }
    }
}
