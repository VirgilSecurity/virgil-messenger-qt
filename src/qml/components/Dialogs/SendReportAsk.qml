import QtQuick 2.15
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.15

TemplateDialog {
    id: sendReportAsk
    title: qsTr("Send report")
    text: qsTr("Previous run crashed. Send report?")
    //standardButtons: Dialog.Yes | StandardButton.No
    property string acceptedButtonText: qsTr('Yes')
    property string rejectedButtonText: qsTr('No')

    onAccepted: crashReporter.sendLogFiles()
}
