// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12

// Local
import Components.Texts 1.0 as MegaTexts
import Components.BusyIndicator 1.0 as MegaBusyIndicator
import Components.Images 1.0 as MegaImages
import Common 1.0

Qml.RoundButton {
    id: button

    property Colors colors: Colors{}
    property Icon icons: Icon{}
    property Progress progress: Progress{}

    readonly property int focusBorderRadius: 11
    readonly property int focusBorderWidth: 3

    Timer {
        id: busyTimer
        interval: 500;
        running: false;
        repeat: false;
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
        if(!button.enabled && !icons.busyIndicatorVisible)
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
        if(!button.enabled && !icons.busyIndicatorVisible)
        {
            return colors.disabled;
        }
        return colors.background;
    }

    function getTextColor()
    {
        if(button.pressed)
        {
            return colors.textPressed;
        }
        if(button.hovered)
        {
            return colors.textHover;
        }
        if(!button.enabled && !icons.busyIndicatorVisible)
        {
            return colors.textDisabled;
        }
        return colors.text;
    }

    bottomPadding: 8 + focusBorderWidth
    topPadding: 8 + focusBorderWidth
    leftPadding: 16 + focusBorderWidth
    rightPadding: 16 + focusBorderWidth
    height: 36 + 2 * focusBorderWidth
    Layout.preferredHeight: 36 + 2 * focusBorderWidth

    contentItem: Row {

        spacing: 8

        Loader {
            id: leftLoader
            anchors.verticalCenter: parent.verticalCenter
        }

        MegaTexts.Text {
            id: buttonText

            anchors.verticalCenter: parent.verticalCenter
            text: button.text
            color: getTextColor()
            font {
                pixelSize: MegaTexts.Text.Size.Medium
                weight: Font.DemiBold
            }
        }

        Loader {
            id: rightLoader
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    background: Rectangle {
        id: focusRect

        color: "transparent"
        border.color: button.enabled ? (button.focus ? Styles.focus : "transparent") : "transparent"
        border.width: button.focusBorderWidth
        radius: button.focusBorderRadius
        height: button.height

        Rectangle {
        id: backgroundRect
        color: getBackgroundColor()
        anchors.top: focusRect.top
        anchors.left: focusRect.left
        anchors.topMargin: button.focusBorderWidth
        anchors.leftMargin: button.focusBorderWidth
        width: button.width - 2 * button.focusBorderWidth
        height: button.height - 2 * button.focusBorderWidth

        border.width: 2
        border.color: getBorderColor()
        radius: 6
       // opacity: button.enabled || icons.busyIndicatorVisible? 1.0 : 0.5

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Item {
                width: backgroundRect.width
                height: backgroundRect.height
                Rectangle
                {
                    anchors.centerIn: parent
                    width:  backgroundRect.width
                    height: backgroundRect.height
                    radius: 6
                }
            }
        }

        Loader{
            id: backgroundLoader
        }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: {mouse.accepted = false;}
        cursorShape: Qt.PointingHandCursor
    }

    Component {
        id: image
        MegaImages.SvgImage {
            //visible: button.iconSource.length
            source: icons.source
            color: button.enabled ? icons.color : icons.disabledColor
            sourceSize: icons.size
            disabledOpacity: 1
        }
    }
    Component {
        id: busyIndicator
        MegaBusyIndicator.BusyIndicator {
            imageSource: Images.loader
            disabledOpacity: 1
            color: icons.color
        }
    }
}

