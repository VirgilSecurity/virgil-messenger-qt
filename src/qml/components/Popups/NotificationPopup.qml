import QtQuick 2.15

Popup {
    id: root

    QtObject {
        id: d

        readonly property int interval: 3000

        function showPopup(message, popupBackgroundColor, textColor, interval) {
            root.popupBackgroundColor = popupBackgroundColor
            root.popupColorText = textColor
            root.popupText = message
            root.popupInterval = interval
            root.open()
        }
    }

    function showError(message) { d.showPopup(message, "#b44", "#ffffff", d.interval) }
    function showInform(message) { d.showPopup(message, "#FFFACD", "#00", d.interval) }
    function showSuccess(message) { d.showPopup(message, "#66CDAA", "#00", d.interval) }
}
