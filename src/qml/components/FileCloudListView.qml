import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"

ModelListView {
    id: fileCloudListView
    model: models.fileCloud.proxy
    emptyIcon: "../resources/icons/Chats.png"
    emptyText: qsTr("Add a file<br/>by pressing the plus<br/>button above")

    delegate: ListDelegate {
        width: fileCloudListView.width
        backgroundColor: (model.isSelected || down) ? Theme.contactPressedColor : "transparent"

        ImageButton {
            image: model.isDir ? "Folder-Big" : "File-Big"
            imageSize: 48
            iconSize: 40
            onClicked: models.fileCloud.selection.toggle(model.index)
        }

        Column {
            Layout.fillWidth: true
            clip: true

            Text {
                color: Theme.primaryTextColor
                font.pointSize: UiHelper.fixFontSz(15)
                text: model.fileName
                width: parent.width
            }
        }

        Column {
            width: 30
            spacing: 5

            Text {
                text: model.displayFileSize
                color: Theme.primaryTextColor
                font.pointSize: UiHelper.fixFontSz(12)
                anchors.right: parent.right
            }

            Text {
                text: model.displayDateTime
                color: Theme.secondaryTextColor
                font.pointSize: UiHelper.fixFontSz(9)
                anchors.right: parent.right
            }
        }

        onClicked: controllers.fileCloud.processClick(model.index)
    }

    onPlaceholderClicked: attachmentPicker.open(AttachmentTypes.file)
}
