import QtQuick 2.12
import QtQuick.Controls 2.12

import "../"
import "../../theme"

Popup {
    id: templateDialog

    // Popup background settings
    property color backgroundColor: Theme.mainBackgroundColor
    property real backgroundOpacity: 1
    property real backgroundRadius: 12

    // Bahavior settings
    property real enterTransitionDuration: 250
    property real exitTransitionDuration: 200
    property real popupImplicitWidth: 420

    // Content settings
    property string title: "Dialog"
    property string text: "Text"

    // Button settings
    property string acceptedButtonText: qsTr('Ok')
    property string rejectedButtonText: qsTr('Cancel')

    // Signals
    signal accepted()
    signal rejected()

    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose
    padding: 20

    enter: Transition {
        ParallelAnimation {
            NumberAnimation {property: "opacity"; from: 0.0; to: 1.0; easing.type: Easing.InOutQuad;
                duration: enterTransitionDuration; easing.overshoot: 1}
            NumberAnimation {property: "scale"; from: 0.8; to: 1.0; easing.type: Easing.OutBack;
                duration: enterTransitionDuration; easing.overshoot: 2}
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation {property: "opacity"; from: 1.0; to: 0.0; easing.type: Easing.InOutQuad;
                duration: exitTransitionDuration; easing.overshoot: 1}
            NumberAnimation {property: "scale"; from: 1.0; to: 0.8; easing.type: Easing.InOutQuad;
                duration: exitTransitionDuration; easing.overshoot: 1}
        }
    }

    Rectangle {
        focus: false
        color: backgroundColor
        opacity: backgroundOpacity
        radius: backgroundRadius
        anchors.centerIn: parent
        implicitWidth: popupImplicitWidth + 20
        height: contentColumn.height + 40

        Column {
            id: contentColumn
            anchors {
                left: parent.left
                leftMargin: 20
                right: parent.right
                rightMargin: 20
                top: parent.top
                topMargin: 20
            }

            spacing: 26

            Text {
                width: parent.width
                text: templateDialog.title
                color: Theme.primaryTextColor
                font.pointSize: UiHelper.fixFontSz(17)
                wrapMode: Text.WordWrap
            }

            Text {
                width: parent.width
                text: templateDialog.text
                color: Theme.primaryTextColor
                font.pointSize: UiHelper.fixFontSz(15)
                wrapMode: Text.WordWrap
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 26

                FormPrimaryButton {
                    text: acceptedButtonText
                    onClicked: {
                        accepted()
                        templateDialog.close()
                    }
                }

                FormSecondaryButton {
                    text: rejectedButtonText
                    onClicked: {
                        rejected()
                        templateDialog.close()
                    }
                }
            }
        }
    }

    background: Item {}
}
