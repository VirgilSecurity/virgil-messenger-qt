import QtQuick 2.15

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
                attachmentType: AttachmentTypes.file
                onAccepted: picked(fileUrls, AttachmentTypes.file)
            }

            SelectAttachmentsDialog {
                id: selectPictureDialog
                // NOTE(fpohtmeh): picture dialog doesn't work in Ios simulator
                attachmentType: app.isIosSimulator() ? AttachmentTypes.file : AttachmentTypes.picture
                onAccepted: picked(fileUrls, AttachmentTypes.picture)
            }

            Connections {
                target: loader
                function onOpen(attachmentType) {
                    var dialog = (attachmentType === AttachmentTypes.picture) ? selectPictureDialog : selectFileDialog
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
