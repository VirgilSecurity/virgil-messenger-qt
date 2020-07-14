import QtQuick 2.12
import QtQuick.Dialogs 1.1

MessageDialog {
    title: "Send crash report ?"
    icon: StandardIcon.Question
    text: "Previus application run is crashed"
    detailedText: "Send crash report ?"
    standardButtons: StandardButton.Yes | StandardButton.No
    Component.onCompleted: visible = true
    onYes: console.log("copied")
    onNo: console.log("didn't copy")
}