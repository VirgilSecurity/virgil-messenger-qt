import QtQuick 2.12
import com.virgilsecurity.messenger 1.0

import "../base"

Loader {
    id: loader

    signal open(var attachmentType)
    signal picked(var fileUrls, var attachmentType)

    Component {
        id: iosPicker

        Item {
            SelectAttachmentsDialog {
                id: selectFileDialog
                attachmentType: Enums.AttachmentType.File
                onAccepted: picked(fileUrls, attachmentType)
            }

            SelectAttachmentsDialog {
                id: selectPictureDialog
                attachmentType: Enums.AttachmentType.Picture
                onAccepted: picked(fileUrls, attachmentType)
            }

            Connections {
                target: loader
                function onOpen(attachmentType) {
                    var dialog = (attachmentType == Enums.AttachmentType.Picture) ? selectPictureDialog : selectFileDialog
                    dialog.open()
                }
            }
        }
    }

    Component {
        id: defaultPicker

        Item {
            SelectAttachmentsDialog {
                id: selectDialog
                onAccepted: picked(fileUrls, attachmentType)
            }

            Connections {
                target: loader
                function onOpen(attachmentType) {
                    selectDialog.attachmentType = attachmentType
                    selectDialog.open()
                }
            }
        }
    }

    sourceComponent: Platform.isIos ? iosPicker : defaultPicker
}
