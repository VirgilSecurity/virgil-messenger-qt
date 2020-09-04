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
        title: qsTr("Add person")
    }

    Form {
        id: form
        formSpacing: 25

        UserNameFormInput {
            id: username
            label: qsTr("Username")

            Keys.onPressed: {
                if (Platform.isDesktop) {
                    if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return)
                        accept();
                    else if (event.key == Qt.Key_Escape)
                        reject();
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
            enabled: username.acceptableInput
            onClicked: accept()
        }
    }

    footer: Footer { }

    function accept() {
        try {
            form.showLoading(qsTr("Adding of contact..."))
            var future = Messenger.addContact(contact)
            Future.onFinished(future, function(value) {
                form.hideLoading();

                if (Future.result(future) === Result.MRES_OK) {
                    mainView.showChatWith(contact, true)
                }
                else {
                    showPopupError(qsTr("User not found"))
                }
            })
        } catch (error) {
            form.hideLoading();
            console.error(error)
        }
    }

    function reject() {
        mainView.back()
    }
}

