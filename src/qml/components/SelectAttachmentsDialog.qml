import QtQuick 2.0
import QtQuick.Dialogs 1.3

FileDialog {
    id: root
    property int type: 0

    readonly property var _titles: [
        qsTr("Send images"),
        qsTr("Send videos"),
        qsTr("Send audio files"),
        qsTr("Send documents"),
        qsTr("Send files")
    ]
    readonly property var _nameFilters: [
        "Images (*.jpg *.jpeg *.png *.gif)",
        "Videos (*.mp4 *.mkv *.avi *.webm)",
        "Audio files (*.mp3 *.wav *.flac *.ogg *.m4a *.mka)",
        "Documents (*.doc *.docx *.odt)",
        "All files (*)"
    ]
    readonly property var _folders: [
        shortcuts.pictures,
        shortcuts.movies,
        shortcuts.music,
        shortcuts.documents,
        shortcuts.home
    ]

    title: _titles[type]
    nameFilters: _nameFilters[type]
    folder: _folders[type]

    sidebarVisible: false
}
