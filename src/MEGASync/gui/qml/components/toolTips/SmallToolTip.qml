import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

Popup {
    id: root

    property string text: ""

    readonly property int verticalOffset: 10
    readonly property int verticalContentMargin: 4
    readonly property int horizontalContentMargin: 6
    readonly property int radius: 4
    readonly property int underArrowSize: 12
    readonly property int underArrowRadius: 2
    readonly property int underArrowVerticalOffset: 8
    readonly property int underArrowRotation: 45

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
        radius: root.radius

        Rectangle {
            id: underArrow

            color: ColorTheme.buttonPrimary
            width: underArrowSize
            height: underArrowSize
            radius: underArrowRadius
            x: parent.width/2 - width/2
            y: parent.height - underArrowVerticalOffset
            rotation: underArrowRotation
            transform: Scale {
                yScale: 1.8
                origin.y: parent.height / 2
            }
        }
    }

    contentItem: Text {
        text: root.text
        color: ColorTheme.textInverse
    }
}

