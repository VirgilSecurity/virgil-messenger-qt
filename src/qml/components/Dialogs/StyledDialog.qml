import QtQuick 2.15
import QtQuick.Controls 2.15

import "../" // components
import "../../theme"

Popup {
    id: dialog
    anchors.centerIn: parent
    padding: Theme.margin
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose

    property alias title: titleLabel.text
    property alias acceptButtonText: acceptButton.text
    property alias rejectButtonText: rejectButton.text
    property alias acceptButtonEnabled: acceptButton.enabled
    default property alias children: childrenLayout.children

    signal accepted()
    signal rejected()

    QtObject {
        id: d
        readonly property real radius: 10
        readonly property real popupImplicitWidth: Math.min(420, mainView.parent.width * 0.8) // TODO(fpohtmeh): remove trick with mainView
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; easing.type: Easing.InOutQuad; duration: Theme.animationDuration; easing.overshoot: 1 }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; easing.type: Easing.InOutQuad; duration: Theme.animationDuration; easing.overshoot: 1 }
        }
    }

    Rectangle {
        anchors.centerIn: parent
        implicitWidth: d.popupImplicitWidth
        height: contentColumn.height + 2 * Theme.margin
        color: Theme.mainBackgroundColor
        radius: d.radius

        Column {
            id: contentColumn
            anchors {
                left: parent.left
                leftMargin: Theme.margin
                right: parent.right
                rightMargin: Theme.margin
                top: parent.top
                topMargin: Theme.margin
            }

            spacing: Theme.spacing

            Text {
                id: titleLabel
                width: parent.width
                color: Theme.primaryTextColor
                font.pointSize: UiHelper.fixFontSz(17)
                wrapMode: Text.WordWrap
            }

            Column {
                id: childrenLayout
                width: parent.width
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.spacing

                FormSecondaryButton {
                    id: rejectButton
                    text: qsTr("Cancel")
                    onClicked: dialog.rejected()
                }

                FormPrimaryButton {
                    id: acceptButton
                    text: qsTr("Ok")
                    onClicked: dialog.accepted()
                }
            }
        }
    }

    background: Item {}

    onAccepted: dialog.close()
    onRejected: dialog.close()
}
