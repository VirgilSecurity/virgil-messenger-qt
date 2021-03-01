import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../theme"

ModelListView {
    id: cloudFileListView
    model: d.model.proxy
    emptyIcon: "../resources/icons/Chats.png"
    emptyText: qsTr("Add a file<br/>by pressing the menu<br/>button above")

    QtObject {
        id: d
        readonly property real defaultChatHeight: 60
        readonly property var model: models.cloudFiles
        readonly property var selection: model.selection
        readonly property var controller: controllers.cloudFiles
    }

    ListStatusButton {
        text: qsTr("Updating...")
        visible: d.controller.isListUpdating
    }

    delegate: ListDelegate {
        id: fileListDelegate
        width: cloudFileListView.width
        height: d.defaultChatHeight

        Image {
            source: "../resources/icons/%1.png".arg(model.isFolder ? "Folder-Big" : "File-Big")
            width: 40
            height: width
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

        onOpenItem: {
            if (model.isFolder) {
                d.controller.switchToFolder(model.index)
            }
            else {
                d.controller.openFile(model.index)
            }
        }

        onSelectItem: {
            d.selection.multiSelect = multiSelect
            d.selection.toggle(model.index)
        }
    }

    onPlaceholderClicked: attachmentPicker.open(AttachmentTypes.file, true)
}
