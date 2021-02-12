import QtQuick 2.15

MessageDialog {
    id: root
    acceptButtonText: qsTr("Accept")
    rejectButtonText: qsTr("Reject")

    onAccepted: controllers.chats.acceptGroupInvitation()
    onRejected: controllers.chats.rejectGroupInvitation()

    function openWithParams(groupTitle, helloText) {
        title = qsTr("You are invited to group '%1'").arg(groupTitle)
        text = qsTr("Message: %1").arg(helloText)
        open()
    }
}
