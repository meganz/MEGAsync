import QtQuick 2.15
import QtGraphicalEffects 1.15

import common 1.0

Button {
    id: button

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
        if(button.pressed || button.down) {
            return colors.borderPressed;
        } else if(button.checked) {
            return colors.borderSelected;
        } else if(button.hovered) {
            return  colors.borderHover;
        } else if(!button.enabled) {
            return colors.borderDisabled;
        }
        return colors.border;
    }

    function getBackgroundColor() {
        if(button.pressed || button.down) {
            return colors.pressed;
        } else if(button.hovered) {
            return colors.hover;
        } else if(button.checked) {
            return colors.background;
        } else if(!button.enabled) {
            return colors.disabled;
        }
        return colors.background;
    }

    function setFocus(focus)    {
        button.focus = focus;

        if(focus) {
            button.forceActiveFocus();
        }
    }

    checkable: true
    checked: false
    autoExclusive : true

    background: Rectangle {
        id: focusRect

        color: "transparent"
        border.color: button.enabled ? (button.activeFocus ? Styles.focus : "transparent") : "transparent"
        border.width: button.focusBorderWidth
        radius: button.focusBorderRadius

        Rectangle {
            id: buttonBackground

            readonly property int borderRadius: 6

            border.width: (button.pressed || button.down || button.hovered || button.checked) ? 2 : 1
            radius: borderRadius
            color: getBackgroundColor()
            border.color: getBorderColor()

            anchors.top: focusRect.top
            anchors.left: focusRect.left
            anchors.topMargin: button.focusBorderWidth
            anchors.leftMargin: button.focusBorderWidth
            width: button.width - 2 * button.focusBorderWidth
            height: button.height - 2 * button.focusBorderWidth
        }

        DropShadow {
            anchors.fill: buttonBackground
            horizontalOffset: 0
            verticalOffset: 5
            radius: 5.0
            samples: 11
            cached: true
            color: "#0d000000"
            source: buttonBackground
            visible: !button.hovered
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onPressed: {
            mouse.accepted = false;
        }
    }
}
