import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

import common 1.0

import components.texts 1.0 as Texts

Qml.RadioButton {
    id: root

    property RadioButtonColors colors: RadioButtonColors {}
    property RadioButtonSizes sizes: RadioButtonSizes {}

    height: Math.max(indicator.implicitHeight, contentItem.implicitHeight)
    width: indicator.implicitWidth + contentItem.implicitWidth + 2 * sizes.focusBorderWidth

    function getColor() {
        if (root.pressed) {
            return colors.pressed;
        }
        else if (root.hovered) {
            return colors.hover;
        }
        else if (!root.enabled) {
            return colors.disabled;
        }
        return colors.enabled;
    }

    function getSize() {
        if (root.pressed) {
            return sizes.internalCircleWidthPressed;
        }
        else if (root.hovered) {
            return sizes.internalCircleWidthHover;
        }
        else {
            return sizes.internalCircleWidth;
        }
    }

    indicator: Rectangle {
        id: focusCircle

        implicitWidth: sizes.focusWidth
        implicitHeight: focusCircle.implicitWidth
        radius: implicitWidth / 2
        border {
            width: sizes.focusBorderWidth
            color: root.focus ? colors.focus : "transparent"
        }

        Rectangle {
            id: externalCircle

            anchors.centerIn: parent
            implicitWidth: sizes.externalCircleWidth
            implicitHeight: externalCircle.implicitWidth
            radius: implicitWidth / 2
            border {
                width: sizes.externalBorderWidth
                color: getColor()
            }

            Rectangle {
                id: internalCircle

                anchors.centerIn: parent
                implicitWidth: getSize()
                implicitHeight: internalCircle.implicitWidth
                radius: implicitWidth / 2
                color: getColor()
                visible: root.checked
            }
        }
    }

    contentItem: Texts.Text {
        id: textItem

        anchors {
            left: indicator.right
            right: root.right
            leftMargin: sizes.spacing
            verticalCenter: parent.verticalCenter
        }

        text: root.text
        verticalAlignment: Text.AlignVCenter

        MouseArea {
            id: mouseAreaText

            anchors.fill: parent
            onPressed: { mouse.accepted = false; }
            cursorShape: Qt.PointingHandCursor
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
            root.checked = true;
            event.accepted = true;
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        width: parent.width
        onPressed: { mouse.accepted = false; }
        cursorShape: Qt.PointingHandCursor
    }

}
