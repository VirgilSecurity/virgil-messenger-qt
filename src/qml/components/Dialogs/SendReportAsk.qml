import QtQuick 2.12
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.5

Dialog {
    id: sendReportAsk
    title: qsTr("Send report")
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Yes | StandardButton.No

    Label {
        text: qsTr("Previus run crashed. Send report?")
    }

    onAccepted: crashReporter.sendLogFiles()
}
