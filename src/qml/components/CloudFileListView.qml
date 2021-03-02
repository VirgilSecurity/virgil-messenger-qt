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
    }

    ListStatusButton {
        text: qsTr("Updating...")
        visible: d.controller.isListUpdating
    }

    delegate: ListDelegate {
        id: listDelegate
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

                Image {
                    source: "../resources/icons/Plus.png"
                    fillMode: Image.PreserveAspectFit
                    width: 0.4 * parent.iconSize
                    height: width
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: 2
                    visible: model.isShared
                }
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
            height: imageSize

            ContextMenu {
                id: contextMenu
                dropdown: true

                ContextMenuItem {
                    text: qsTr("Open file")
                    visible: !model.isFolder
                    onTriggered: listDelegate.openItem()
                }

                ContextMenuItem {
                    text: qsTr("Open folder")
                    visible: model.isFolder
                    onTriggered: listDelegate.openItem()
                }

                ContextMenuItem {
                    id: sharingItem
                    text: qsTr("Sharing")
                    visible: model.isShared
                    onTriggered: appState.requestSharingInfo()
                }

                ContextMenuSeparator {
                }

                ContextMenuItem {
                    text: qsTr("Delete")
                    onTriggered: deleteCloudFilesDialog.open()
                }
            }

            onClicked: {
                d.selection.clear()
                d.selection.toggle(model.index)
                contextMenu.open()
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
