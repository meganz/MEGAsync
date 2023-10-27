// System
import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

// Local
import Components.Texts 1.0 as MegaTexts
import Components.BusyIndicator 1.0 as MegaBusyIndicator
import Components.Images 1.0 as MegaImages
import Common 1.0

Qml.RoundButton {
    id: root

    property Colors colors: Colors {}
    property Icon icons: Icon{}
    property alias progressValue: backgroundProgress.value
    property Sizes sizes: Sizes {}

    function getBorderColor() {
        if(root.pressed || root.down || root.checked) {
            return colors.borderPressed;
        }
        if(root.hovered) {
            return colors.borderHover;
        }
        if(!root.enabled && !icons.busyIndicatorVisible) {
            return colors.borderDisabled;
        }
        return colors.border;
    }

    function getBackgroundColor() {
        if(root.pressed || root.down || root.checked) {
            return colors.pressed;
        }
        if(root.hovered) {
            return colors.hover;
        }
        if(!root.enabled && !icons.busyIndicatorVisible) {
            return colors.disabled;
        }
        return colors.background;
    }

    function getTextColor() {
        if(root.pressed || root.down || root.checked) {
            return colors.textPressed;
        }
        if(root.hovered) {
            return colors.textHover;
        }
        if(!root.enabled && !icons.busyIndicatorVisible) {
            return colors.textDisabled;
        }
        return colors.text;
    }

    function getIconColor() {
        if(root.pressed || root.down || root.checked) {
            return icons.colorPressed;
        }
        if(root.hovered) {
            return icons.colorHovered;
        }
        if(!root.enabled && !icons.busyIndicatorVisible) {
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

        MegaImages.SvgImage {
            id: leftImage

            anchors.verticalCenter: parent.verticalCenter
            source: root.icons.source
            color: getIconColor()
            sourceSize: sizes.iconSize
            visible: root.icons.position === Icon.Position.LEFT && !root.icons.busyIndicatorVisible
        }

        MegaBusyIndicator.BusyIndicator {
            id: leftBusyIndicator
            anchors.verticalCenter: parent.verticalCenter

            color: root.icons.colorEnabled
            visible: root.icons.position === Icon.Position.LEFT && root.icons.busyIndicatorVisible
        }

        MegaTexts.Text {
            id: buttonText

            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: getTextColor()
            font {
                pixelSize: sizes.textFontSize
                weight: Font.DemiBold
            }
        }

        MegaImages.SvgImage {
            id: rightImage

            anchors.verticalCenter: parent.verticalCenter
            source: root.icons.source
            color: getIconColor()
            sourceSize: sizes.iconSize
            visible: root.icons.position === Icon.Position.RIGHT && !root.icons.busyIndicatorVisible
        }

        MegaBusyIndicator.BusyIndicator {
            id: rightBusyIndicator

            anchors.verticalCenter: parent.verticalCenter
            color: icons.colorEnabled
            visible: root.icons.position === Icon.Position.RIGHT && root.icons.busyIndicatorVisible
        }
    }

    background: Rectangle {
        id: focusRect

        color: "transparent"
        border.color: root.enabled ? (root.activeFocus ? Styles.focus : "transparent") : "transparent"
        border.width: sizes.focusBorderWidth
        radius: sizes.focusBorderRadius
        height: root.height

        Rectangle {
            id: backgroundRect

            color: getBackgroundColor()
            anchors.top: focusRect.top
            anchors.left: focusRect.left
            anchors.topMargin: sizes.focusBorderWidth
            anchors.leftMargin: sizes.focusBorderWidth
            width: root.width - 2 * sizes.focusBorderWidth
            height: root.height - 2 * sizes.focusBorderWidth
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

            Progress {
                id: backgroundProgress
                anchors.fill: parent
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: { mouse.accepted = false; }
        cursorShape: Qt.PointingHandCursor
    }

    Keys.onReleased: (event)=> {
        if (event.key === Qt.Key_Return) {
            if (checkable) {
                down = false;
                checked = true;
            }
            event.accepted = true;
        }
    }

    Keys.onReturnPressed: {
        down = true;
        root.clicked();
    }

    Keys.onEnterPressed:{
        root.clicked();
    }

}
