import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "./components"
import "./pages"

Control {
    id: mainView

    ColumnLayout {
        anchors {
            fill: parent
            bottomMargin: keyboardHandler.keyboardHeight
        }
        spacing: 0

        MainStackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        LogControl {
            visible: settings.devMode
            Layout.fillWidth: true
            Layout.preferredHeight: 0.25 * mainView.height
        }
    }

    function navigateBack(transition) { return stackView.navigateBack(transition) }
}
