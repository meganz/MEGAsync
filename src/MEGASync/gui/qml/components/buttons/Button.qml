// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Components 1.0
import Common 1.0

Qml.RoundButton {
    id: button

    property bool primary: false
    property string iconSource: ""
    property size iconSize: Qt.size(16, 16)

    bottomPadding: 8
    topPadding: 8
    leftPadding: 16
    rightPadding: 16

    contentItem: RowLayout {
        width: button.width
        height: button.height
        spacing: 8

        Text {
            id: textButton

            color: primary ? Styles.textOnColor : Styles.buttonPrimary
            text: button.text
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font {
                pixelSize: 14
                weight: Font.DemiBold
            }
        }

        SvgImage {
            source: button.iconSource
            color: Styles.iconOnColor
            sourceSize: button.iconSize
        }
    }

    background: Rectangle {
        width: button.width
        height: button.height
        border.width: 2
        radius: 6
        color: primary ? Styles.buttonPrimary : "transparent"
        border.color: Styles.buttonPrimary
        opacity: button.enabled ? 1.0 : 0.1
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: mouse.accepted = false
        cursorShape: Qt.PointingHandCursor
    }

}
