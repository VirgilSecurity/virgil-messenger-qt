import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QuickFuture 1.0
import MesResult 1.0

import "../base"
import "../theme"
import "../components"

Page {
    property bool showServersPanel: true
    readonly property string contact: username.text.toLowerCase()

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    header: Header {
        showBackButton: !form.isLoading
        title: qsTr("New chat")
    }

    Form {
        id: form
        formSpacing: 25
        focus: true

        UserNameFormInput {
            id: username
            label: qsTr("Username")
            focus: true

            Keys.onPressed: {
                if (Platform.isDesktop) {
                    if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
                        accept();
                    }
                    else if (event.key == Qt.Key_Escape) {
                        reject();
                    }
                }
            }
        }

        Rectangle {
            height: 1
            color: "gray"
            opacity: 0.5
            Layout.fillWidth: true
            Layout.maximumWidth: Theme.formMaximumWidth
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: Theme.formMaximumWidth
            Layout.alignment: Qt.AlignHCenter

            Label {
                text: qsTr("Server")
                color: Theme.primaryTextColor
                Layout.fillWidth: true
            }

            Label {
                color: Theme.secondaryTextColor
                text: qsTr("Default")
            }
        }

        FormPrimaryButton {
            text: qsTr("Add to contacts")
            onClicked: accept()
        }
    }

    footer: Footer { }

    function accept() {
        app.stateManager.addContact(contact)
    }

    function reject() {
        app.stateManager.goBack()
    }

    Connections {
        target: app.stateManager.newChatState

        function onAddContactStarted(userId) {
            form.showLoading(qsTr("Adding of contact..."))
        }

        function onAddContactFinished() {
            form.hideLoading();
        }

        function onAddContactErrorOccurred(errorText) {
            form.hideLoading();
            showPopupError(errorText) // TODO(fpohtmeh): don't use parent method directly
        }
    }
}

