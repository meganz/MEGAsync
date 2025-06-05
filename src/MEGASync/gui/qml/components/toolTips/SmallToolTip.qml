import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

Popup {
    id: tooltip

    property string text: ""
    readonly property int verticalOffset: 10
    readonly property int verticalContentMargin: 4
    readonly property int horizontalContentMargin: 6
    readonly property int radius: 4

    width: implicitWidth
    height: implicitHeight
    x: parent.width / 2 - width / 2
    y: -height -verticalOffset
    topPadding: verticalContentMargin
    bottomPadding: verticalContentMargin
    leftPadding: horizontalContentMargin
    rightPadding: horizontalContentMargin
    modal: false
    focus: false

    background: Rectangle {
        color: ColorTheme.buttonPrimary
        radius: tooltip.radius

        Rectangle {
            id: underArrow

            color: ColorTheme.buttonPrimary
            width: 12
            height: width
            radius: 2
            x: parent.width/2 - width/2
            y: parent.height - 8
            rotation: 45
            transform: Scale {
                yScale: 1.8
                origin.y: parent.height / 2
            }
        }
    }

    contentItem: Text {
        text: tooltip.text
        color: ColorTheme.textInverse
    }
}

