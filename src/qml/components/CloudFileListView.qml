import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../theme"

ModelListView {
    id: cloudFileListView
    model: models.cloudFiles.proxy
    emptyIcon: "../resources/icons/Chats.png"
    emptyText: qsTr("Add a file<br/>by pressing the menu<br/>button above")

    QtObject {
        id: d
        readonly property real defaultChatHeight: 60
    }

    ListStatusButton {
        text: qsTr("Updating...")
        visible: controllers.cloudFiles.isLoading
    }

    delegate: ListDelegate {
        id: fileListDelegate
        width: cloudFileListView.width
        height: d.defaultChatHeight
        backgroundColor: (model.isSelected || down) ? Theme.contactPressedColor : "transparent"

        ImageButton {
            image: model.isFolder ? "Folder-Big" : "File-Big"
            imageSize: 48
            iconSize: 40
            onClicked: models.cloudFiles.selection.toggle(model.index)
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

        onClicked: {
            if (model.isFolder) {
                controllers.cloudFiles.switchToFolder(model.index)
            }
            else {
                controllers.cloudFiles.openFile(model.index)
            }
        }
    }

    onPlaceholderClicked: attachmentPicker.open(AttachmentTypes.file)
}
