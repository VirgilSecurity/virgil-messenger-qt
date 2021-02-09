import QtQuick 2.15

MessageDialog {
    title: qsTr("Send report")
    text: qsTr("Previous run crashed. Send report?")
    //standardButtons: Dialog.Yes | StandardButton.No // TODO(fpohtmeh): implement
    acceptButtonText: qsTr("Yes")
    rejectButtonText: qsTr("No")
    onAccepted: crashReporter.sendLogFiles("crash-logs from dialog")
}
