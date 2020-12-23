import QtQuick 2.15
import QtQuick.Dialogs 1.3

import "../base"

FileDialog {
    id: root
    property var attachmentType: AttachmentTypes.file

    readonly property var _titles: [
        qsTr("Send files"),
        qsTr("Send images"),
        qsTr("Send videos"),
        qsTr("Send audio files")
    ]
    readonly property var _nameFilters: [
        "All files (*)",
        "Images (*.jpg *.jpeg *.png *.gif)",
        "Videos (*.mp4 *.mkv *.avi *.webm)",
        "Audio files (*.mp3 *.wav *.flac *.ogg *.m4a *.mka)"
    ]
    readonly property var _folders: [
        shortcuts.home,
        shortcuts.pictures,
        shortcuts.movies,
        shortcuts.music,
        shortcuts.documents,
        shortcuts.desktop
    ]

    title: _titles[attachmentType]
    nameFilters: _nameFilters[attachmentType]
    folder: _folders[attachmentType]

    sidebarVisible: true
}
