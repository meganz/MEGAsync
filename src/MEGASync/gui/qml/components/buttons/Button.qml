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

    property Colors colors: Colors {}
    property Icon icons: Icon {}
    property Progress progress: Progress {}
    property Sizes sizes: Sizes {}

    Timer {
        id: busyTimer

        interval: 500;
        running: false;
        repeat: false;
    }

    function getBorderColor() {
        if(button.pressed || button.checked) {
            return colors.borderPressed;
        }
        if(button.hovered) {
            return colors.borderHover;
        }
        if(!button.enabled && !icons.busyIndicatorVisible) {
            return colors.borderDisabled;
        }
        return colors.border;
    }

    function getBackgroundColor() {
        if(button.pressed || button.checked) {
            return colors.pressed;
        }
        if(button.hovered) {
            return colors.hover;
        }
        if(!button.enabled && !icons.busyIndicatorVisible) {
            return colors.disabled;
        }
        return colors.background;
    }

    function getTextColor() {
        if(button.pressed || button.checked) {
            return colors.textPressed;
        }
        if(button.hovered) {
            return colors.textHover;
        }
        if(!button.enabled && !icons.busyIndicatorVisible) {
            return colors.textDisabled;
        }
        return colors.text;
    }

    function getIconColor() {
        if(button.pressed || button.checked) {
            return icons.colorPressed;
        }
        if(button.hovered) {
            return icons.colorHovered;
        }
        if(!button.enabled && !icons.busyIndicatorVisible) {
            return icons.colorDisabled;
        }
        return icons.colorEnabled;
    }

    bottomPadding: sizes.verticalPadding + sizes.focusBorderWidth
    topPadding: sizes.verticalPadding + sizes.focusBorderWidth
    leftPadding: sizes.horizontalPadding + sizes.focusBorderWidth
    rightPadding: sizes.horizontalPadding + sizes.focusBorderWidth
    height: sizes.height + 2 * sizes.focusBorderWidth
    Layout.preferredHeight: sizes.height + 2 * sizes.focusBorderWidth

    contentItem: Row {

        spacing: sizes.spacing

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
                pixelSize: sizes.textFontSize
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
        border.width: sizes.focusBorderWidth
        radius: sizes.focusBorderRadius
        height: button.height

        Rectangle {
            id: backgroundRect

            color: getBackgroundColor()
            anchors.top: focusRect.top
            anchors.left: focusRect.left
            anchors.topMargin: sizes.focusBorderWidth
            anchors.leftMargin: sizes.focusBorderWidth
            width: button.width - 2 * sizes.focusBorderWidth
            height: button.height - 2 * sizes.focusBorderWidth
            border.width: sizes.borderWidth
            border.color: getBorderColor()
            radius: sizes.radius
            layer.enabled: true

            layer.effect: OpacityMask {

                maskSource: Item {
                    width: backgroundRect.width
                    height: backgroundRect.height

                    Rectangle {
                        anchors.centerIn: parent
                        width:  backgroundRect.width
                        height: backgroundRect.height
                        radius: sizes.radius
                    }
                }
            }

            Loader {
                id: backgroundLoader
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: { mouse.accepted = false; }
        cursorShape: Qt.PointingHandCursor
    }

    Component {
        id: image

        MegaImages.SvgImage {
            source: icons.source
            color: getIconColor()
            sourceSize: sizes.iconSize
        }
    }

    Component {
        id: busyIndicator

        MegaBusyIndicator.BusyIndicator {
            imageSource: Images.loader
            color: icons.colorEnabled
        }
    }

    Keys.onReturnPressed: button.clicked()
    Keys.onEnterPressed: button.clicked()
}
