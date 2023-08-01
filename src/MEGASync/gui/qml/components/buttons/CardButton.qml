// System
import QtQuick 2.12
import QtGraphicalEffects 1.0

// QML common
import Common 1.0

Button {
    id: button

    property string title
    property string description
    property string imageSource
    property size imageSourceSize
    property Component contentComponent
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
        if(button.pressed) {
            return colors.borderPressed;
        } else if(button.hovered) {
            return  colors.borderHover;
        } else if(button.checked) {
            return colors.borderSelected;
        } else if(!button.enabled) {
            return colors.borderDisabled;
        }
        return colors.border;
    }

    function getBackgroundColor() {
        if(button.pressed) {
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

    checkable: true
    checked: false
    autoExclusive : true

    background: Rectangle {

        id: focusRect

        readonly property int focusBorderRadius: 8
        readonly property int focusBorderWidth: 4

        color: "transparent"
        border.color: button.enabled ? (button.focus ? Styles.focus : "transparent") : "transparent"
        border.width: focusRect.focusBorderWidth
        radius: focusRect.focusBorderRadius

        Rectangle {
            id: buttonBackground

            readonly property int borderRadius: 6

            border.width: (button.pressed || button.hovered || button.checked) ? 2 : 1
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
        }

        Loader {
            id: contentLoader

            anchors.fill: parent
        }
    }

    onContentComponentChanged: {
        contentLoader.sourceComponent = contentComponent;
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
