import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml

Qml.RoundButton {
    bottomPadding:12
    topPadding: 12
    leftPadding: 24
    rightPadding: 24
    radius:6
    MouseArea
    {
        id: mouseArea
        anchors.fill: parent
        onPressed:  mouse.accepted = false
        cursorShape: Qt.PointingHandCursor
    }
}
