import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

Qml.ToolTip {
    id: root

    property url leftIconSource: ""

    z: 10
    padding: 4

    background: Rectangle {
        id: backgroundRect

        anchors.fill: parent
        color: colorStyle.buttonPrimary
        radius: 4

        SvgImage {
            id: leftIcon

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
                leftMargin: root.padding
            }
            source: leftIconSource
            color: colorStyle.iconOnColor
            sourceSize: Qt.size(16, 16)
        }
    }

    contentItem: Item {
        id: content

        implicitWidth: textToolTip.width + leftIcon.width + root.padding
        implicitHeight: Math.max(leftIcon.width, textToolTip.height)

        Texts.Text {
            id: textToolTip

            property int maxWidth: 778 - leftIcon.width - root.padding

            anchors {
                left: parent.left
                top: parent.top
                leftMargin: leftIcon.width + root.padding
            }
            width: Math.min(textMetrics.width + root.padding, maxWidth)
            text: root.text
            color: colorStyle.textInverse
            wrapMode: Text.Wrap
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
