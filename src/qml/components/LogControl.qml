import QtQuick 2.12
import QtQuick.Controls 2.12

import "../base"

Item {
    id: logControl
    anchors {
        fill: parent
        topMargin: 0.75 * mainView.height
    }
    visible: settings.devMode

    TextScrollView {
        anchors.fill: parent

        TextArea {
            id: logControlTextArea
            width: mainView.width
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            textFormat: Text.RichText
            readOnly: true
            font.pointSize: Platform.isMobile ? 12 : 9
            selectByMouse: Platform.isDesktop

            background: Rectangle {
                color: "white"
            }

            Component.onCompleted: {
                if (logControl.visible) {
                    logging.formattedMessageCreated.connect(append)
                }
            }
        }
    }

    Button {
        anchors.right: parent.right
        anchors.top: parent.top
        text: "x"
        width: 20
        height: width
        onClicked: logControlTextArea.clear()
    }
}
