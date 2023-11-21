import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

import common 1.0

import components.texts 1.0
import components.images 1.0

Qml.ToolTip {
    id: root

    property url leftIconSource: ""

    z: 10
    padding: 4

    background: Rectangle {
        anchors.fill: parent
        color: Styles.buttonPrimary
        radius: 4

        SvgImage {
            id: leftIcon

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: root.padding
            source: leftIconSource
            color: Styles.iconOnColor
            sourceSize: Qt.size(16, 16)
        }
    }

    contentItem: Item {
        implicitWidth: textToolTip.width + leftIcon.width + root.padding
        implicitHeight: Math.max(leftIcon.width, textToolTip.height)

        Text {
            id: textToolTip

            property int maxWidth: 778 - leftIcon.width - root.padding

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: leftIcon.width + root.padding
            text: root.text
            color: Styles.textInverse
            wrapMode: Text.Wrap
            width: Math.min(textMetrics.width + root.padding, maxWidth)
            lineHeight: Math.max(leftIcon.height, textMetrics.height)
            lineHeightMode: Text.FixedHeight
            verticalAlignment: Qt.AlignVCenter

            TextMetrics {
                id: textMetrics

                font: textToolTip.font
                text: textToolTip.text
            }
        }
    }

}
