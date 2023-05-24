// System
import QtQuick 2.12

// QML common
import Common 1.0
import Components 1.0 as Custom

Button {
    id: button

    property string title
    property string description
    property string imageSource
    property size imageSourceSize
    property Component contentComponent
    property Colors colors: Colors{
        background: Styles.pageBackground
        hover: Styles.buttonOutlineBackgroundHover
        pressed: Styles.pageBackground
        border: Styles.borderDisabled
        borderDisabled: Styles.borderDisabled
        borderHover: Styles.borderDisabled
        borderPressed: Styles.borderSubtle
        borderSelected: Styles.borderStrongSelected
    }

    function getBorderColor()
    {
        if(button.pressed)
        {
            return colors.borderPressed;
        }
        if(button.hovered)
        {
            return  colors.borderHover;
        }
        if(button.checked)
        {
            return colors.borderSelected;
        }
        if(!button.enabled)
        {
            return colors.borderDisabled;
        }
        return colors.border;
    }

    function getBackgroundColor()
    {
        if(button.pressed)
        {
            return colors.pressed;
        }
        if(button.hovered)
        {
            return colors.hover;
        }
        if(button.checked)
        {
            return colors.background;
        }
        if(!button.enabled)
        {
            return colors.disabled;
        }
        return colors.background;
    }

    checkable: true
    checked: false
    autoExclusive : true

    background: Rectangle {

        id: focusRect

        readonly property int focusBorderRadius: 10
        readonly property int focusBorderWidth: 3

        color: "transparent"
        border.color: button.enabled ? (button.focus ? Styles.focus : "transparent") : "transparent"
        border.width: focusRect.focusBorderWidth
        radius: focusRect.focusBorderRadius
        height: focusRect.height

        Rectangle{
            id: buttonBackground

            readonly property int borderRadius: 8
            readonly property int borderWidth: 2

            border.width: borderWidth
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
