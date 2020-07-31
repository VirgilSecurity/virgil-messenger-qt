import QtQuick 2.12
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.5

Dialog {
    title: "Send crash report"
    Label {
        text: "Previous run crashed. Send report?"
    }
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Yes | StandardButton.No
    onAccepted: crashReporter.send()
}
