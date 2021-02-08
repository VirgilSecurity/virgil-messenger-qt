import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../base"
import "../theme"

Item {
    height: row.height + 2 * offset
    width: row.width + 2 * offset

    readonly property double offset: d.isPicture ? d.defaultRadius : 12

    Row {
        id: row
        spacing: 14
        x: offset
        y: offset

        Rectangle {
            id: imageRect
            y: 0.5 * (row.height - height)
            width: image.width
            height: image.height
            color: d.isPicture && model.attachmentIconPath ? "white" : "transparent"

            Image {
                id: image
                Binding on width {
                    when: d.isPicture
                    value: image.pictureWidth
                }
                Binding on height {
                    when: d.isPicture
                    value: image.pictureWidth * attachmentPictureThumbnailHeight / attachmentPictureThumbnailWidth
                }
                autoTransform: true
                visible: d.isPicture ? true : model.attachmentFileExists && !progressBar.visible
                source: model.attachmentIconPath

                readonly property double pictureWidth: d.isPicture ? Math.min(3 * attachmentPictureThumbnailWidth, maxWidth - 2 * offset) : 0
            }

            Rectangle {
                visible: !model.attachmentFileExists && !progressBar.visible
                anchors.centerIn: parent
                width: 1.3333 * downloadImage.width
                height: width
                radius: 0.5 * width
                color: "#364542"

                Image {
                    id: downloadImage
                    anchors.centerIn: parent
                    source: "../resources/icons/File Download Big.png"
                }
            }

            CircleProgressBar {
                id: progressBar
                anchors.centerIn: parent
                size: 40
                visible: model.attachmentIsLoading
                maxValue: Math.max(1, model.attachmentBytesTotal)
                value: Math.min(0.99 * maxValue, Math.max(0.01 * maxValue, model.attachmentBytesLoaded))
                animated: visible
            }
        }

        ColumnLayout {
            id: column
            spacing: 4
            visible: !d.isPicture
            readonly property double maxWidth: chatMessage.maxWidth - imageRect.width - row.spacing - 2 * offset

            Label {
                Layout.fillHeight: true
                Layout.maximumWidth: column.maxWidth
                text: model.attachmentDisplayText
                color: "white"
                font.pixelSize: UiHelper.fixFontSz(16)
                wrapMode: Text.Wrap
            }

            Label {
                Layout.maximumWidth: column.maxWidth
                visible: model.attachmentIsLoaded
                text: model.attachmentDisplaySize
                color: "white"
                font.pixelSize: UiHelper.fixFontSz(10)
            }

            Label {
                Layout.maximumWidth: column.maxWidth
                visible: model.attachmentIsLoading
                text: model.attachmentDisplayProgress
                color: "white"
                font.pixelSize: UiHelper.fixFontSz(10)
            }
        }
    }

    property var contextMenu: ContextMenu {
        compact: true
        enabled: !model.isBroken

        Action {
            text: Platform.isMobile ? qsTr("Save to downloads") : qsTr("Save As...")
            onTriggered: (Platform.isMobile ? controllers.attachments.download : chatMessage.saveAttachmentAs)(messageId)
        }
    }
}
