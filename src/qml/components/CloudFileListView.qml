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
        readonly property var model: models.cloudFiles
        readonly property var selection: model.selection
        readonly property var controller: controllers.cloudFiles

        property var contextMenu: ContextMenu {
            dropdown: true

//            ContextMenuItem {
//                text: qsTr("Item 1")
//            }

//            ContextMenuItem {
//                text: qsTr("Item 1")
//            }
        }
    }

    ListStatusButton {
        text: qsTr("Updating...")
        visible: d.controller.isListUpdating
    }

    delegate: ListDelegate {
        id: fileListDelegate
        width: cloudFileListView.width
        height: Theme.headerHeight
        rightMargin: 0
        selectionModel: d.selection

        Item {
            Layout.preferredWidth: Theme.avatarWidth
            Layout.preferredHeight: Theme.avatarHeight

            Image {
                source: "../resources/icons/%1.png".arg(model.isFolder ? "Folder-Big" : "File-Big")
                anchors.centerIn: parent
            }
        }

        TwoLineLabel {
            Layout.fillWidth: true
            clip: true
            title: model.fileName
            description: model.isFolder ? "" : model.displayFileSize
        }

        ImageButton {
            id: menuButton
            image: "More"
            onClicked: {
                d.contextMenu.parent = menuButton
                d.contextMenu.open()
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
