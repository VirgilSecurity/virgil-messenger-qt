import QtQuick 2.12
import QtQuick.Controls 2.12

TextField {
    inputMethodHints: Qt.ImhNoAutoUppercase
    validator: app.validator.reUsername
}
