import QtQuick 2.15

import "../base"
import "../components/Dialogs"

Loader {
    id: root

    signal open(int attachmentType, bool selectMultiple)
    signal picked(var fileUrls, int attachmentType)

    signal opening()
    signal closed()

    Component {
        id: iosPicker

        Item {
            SelectAttachmentsDialog {
                id: selectFileDialog
                attachmentType: AttachmentTypes.file
                onAccepted: {
                    root.picked(fileUrls, AttachmentTypes.file)
                    root.closed()
                }
                onRejected: root.closed()
            }

            SelectAttachmentsDialog {
                id: selectPictureDialog
                // NOTE(fpohtmeh): picture dialog doesn't work in Ios simulator
                attachmentType: app.isIosSimulator() ? AttachmentTypes.file : AttachmentTypes.picture
                onAccepted: {
                    root.picked(fileUrls, AttachmentTypes.picture)
                    root.closed()
                }
                onRejected: root.closed()
            }

            Connections {
                target: root
                function onOpen(attachmentType, selectMultiple) {
                    root.opening()
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
                onAccepted: {
                    root.picked(fileUrls, attachmentType)
                    root.closed()
                }
                onRejected: root.closed()
            }

            Connections {
                target: root
                function onOpen(attachmentType, selectMultiple) {
                    root.opening()
                    selectDialog.attachmentType = attachmentType
                    selectDialog.selectMultiple = selectMultiple
                    selectDialog.open()
                }
            }
        }
    }

    sourceComponent: Platform.isIos ? iosPicker : defaultPicker
}
