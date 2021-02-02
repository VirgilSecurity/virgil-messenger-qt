import QtQuick 2.15

import "../base"

Loader {
    id: loader

    signal open(int attachmentType, bool selectMultiple)
    signal picked(var fileUrls, int attachmentType)

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
                function onOpen(attachmentType, selectMultiple) {
                    var dialog = (attachmentType === AttachmentTypes.picture) ? selectPictureDialog : selectFileDialog
                    dialog.selectMultiple = selectMultiple
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
                function onOpen(attachmentType, selectMultiple) {
                    selectDialog.attachmentType = attachmentType
                    selectDialog.selectMultiple = selectMultiple
                    selectDialog.open()
                }
            }
        }
    }

    sourceComponent: Platform.isIos ? iosPicker : defaultPicker
}
