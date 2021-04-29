import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Item {
    id: root
    clip: true
    implicitWidth: column.implicitWidth
    implicitHeight: column.implicitHeight

    property alias title: titleLabel.text
    property alias description: descriptionLabel.text

    property alias horizontalAlignment: titleLabel.horizontalAlignment
    property alias bold: titleLabel.font.bold
    property bool fillWidth: true
    property alias clickable: mouseArea.enabled

    property alias titleLabel: titleLabel
    property alias descriptionLabel: descriptionLabel

    signal clicked()

    Column {
        id: column
        anchors.fill: parent

        Label {
            id: titleLabel
            font.pointSize: UiHelper.fixFontSz(15)
            elide: Label.ElideRight
            color: Theme.primaryTextColor

            Binding on width { when: root.fillWidth; value: column.width }
            Binding on anchors.horizontalCenter { when: horizontalAlignment == Qt.AlignHCenter; value: root.horizontalCenter }
            Binding on font.family { when: root.bold; value: Theme.mainFont }
        }

        Label {
            id: descriptionLabel
            font.pointSize: UiHelper.fixFontSz(11)
            elide: Label.ElideRight
            color: Theme.secondaryTextColor
            horizontalAlignment: titleLabel.horizontalAlignment
            visible: descriptionLabel.text

            Binding on width { when: root.fillWidth; value: column.width }
            Binding on anchors.horizontalCenter { when: horizontalAlignment == Qt.AlignHCenter; value: root.horizontalCenter }
            Binding on font.family { when: root.bold; value: Theme.mainFont }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: false

        onClicked: root.clicked()
    }
}
