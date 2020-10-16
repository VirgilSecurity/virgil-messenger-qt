import QtQuick 2.12
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.5

Dialog {
    id: sendReportAsk
    title: "Send report ?"
    Label {
           text: "Previus run crashed. Send report ?"
    }
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Yes | StandardButton.No
    onAccepted: crashReporter.sendLogFiles()
}
