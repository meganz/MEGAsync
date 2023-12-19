import QtQuick 2.15
import QtGraphicalEffects 1.15

import common 1.0

Button {
    id: root

    readonly property int focusBorderWidth: 4
    readonly property int focusBorderRadius: 10

    property string title
    property string description
    property string imageSource
    property size imageSourceSize

    property Colors colors: Colors {
        background: Styles.pageBackground
        hover: Styles.buttonOutlineBackgroundHover
        pressed: Styles.pageBackground
        border: Styles.borderDisabled
        borderDisabled: Styles.borderDisabled
        borderHover: Styles.borderDisabled
        borderPressed: Styles.borderSubtle
        borderSelected: Styles.borderStrongSelected
    }

    function getBorderColor() {
        if(root.pressed || root.down) {
            return colors.borderPressed;
        }
        else if(root.checked) {
            return colors.borderSelected;
        }
        else if(root.hovered) {
            return  colors.borderHover;
        }
        else if(!root.enabled) {
            return colors.borderDisabled;
        }
        return colors.border;
    }

    function getBackgroundColor() {
        if(root.pressed || root.down) {
            return colors.pressed;
        }
        else if(root.hovered) {
            return colors.hover;
        }
        else if(root.checked) {
            return colors.background;
        }
        else if(!root.enabled) {
            return colors.disabled;
        }
        return colors.background;
    }

    checkable: true
    checked: false
    autoExclusive: true

    background: Rectangle {
        id: focusRect

        color: "transparent"
        radius: root.focusBorderRadius
        border {
            color: root.enabled
                   ? (root.activeFocus ? Styles.focus : "transparent")
                   : "transparent"
            width: root.focusBorderWidth
        }

        Rectangle {
            id: buttonBackground

            readonly property int borderRadius: 6

            anchors {
                top: focusRect.top
                left: focusRect.left
                topMargin: root.focusBorderWidth
                leftMargin: root.focusBorderWidth
            }
            width: root.width - 2 * root.focusBorderWidth
            height: root.height - 2 * root.focusBorderWidth
            radius: borderRadius
            color: getBackgroundColor()
            border {
                width: (root.pressed || root.down || root.hovered || root.checked) ? 2 : 1
                color: getBorderColor()
            }
        }

        DropShadow {
            id: shadow

            anchors.fill: buttonBackground
            horizontalOffset: 0
            verticalOffset: 5
            radius: 5.0
            samples: 11
            cached: true
            color: "#0d000000"
            source: buttonBackground
            visible: !root.hovered
        }

    } // Rectangle: focusRect

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onPressed: {
            mouse.accepted = false;
        }
    }
}
