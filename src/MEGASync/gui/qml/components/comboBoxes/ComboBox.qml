import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtGraphicalEffects 1.15

import common 1.0

import components.buttons 1.0 as Buttons
import components.texts 1.0 as Texts
import components.images 1.0

Qml.ComboBox {
    id: root

    property int popupWidth: -1
    property Sizes sizes: Sizes{}
    readonly property int itemHeight: 40

    function getBackgroundColor(){
        if(itemDelegate.pressed) {
            return ColorTheme.surface2;
        }
        else if(itemRect.hovered) {
            return ColorTheme.textInverse;
        }

        return "transparent";
    }

    implicitHeight: sizes.defaultHeight + 2 * sizes.focusBorderWidth
    font{
        weight: Font.DemiBold
        pixelSize: Texts.Text.Size.MEDIUM
    }
    background: Rectangle {
        id: focusBorderRectangle

        color:"transparent"
        radius: sizes.focusBorderRadius
        border {
            color: root.activeFocus && !popup.visible ? ColorTheme.focusColor : "transparent"
            width: sizes.focusBorderWidth
        }
        Rectangle{
            id: backgroundRectangle

            anchors{
                fill: parent
                margins: sizes.focusBorderWidth
            }
            border.width: sizes.borderWidth
            border.color: enabled === true? ColorTheme.buttonOutline : ColorTheme.buttonDisabled
            radius: sizes.borderRadius
            color: "transparent"
        }
    }
    delegate: Qml.ItemDelegate {
        id: itemDelegate

        width: popup.width - sizes.popupPadding * 2
        height: itemHeight
        padding:1

        background: Item{}
        contentItem: Rectangle{
            id: itemRect

            color: {
                if(itemDelegate.pressed) {
                    return ColorTheme.surface2;
                }
                else if(itemDelegate.hovered) {
                    return ColorTheme.surface1;
                }

                return "transparent";
            }
            radius: 6
            anchors{
                left: parent.left
            }
            border.width: sizes.itemBorderWidth
            border.color: itemDelegate.activeFocus ? ColorTheme.focus : "transparent"
            SvgImage {
                id: leftIcon

                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: 8
                }
                visible: root.currentIndex === index
                source: Images.check
                color: ColorTheme.supportSuccess
                sourceSize: Qt.size(16, 16)
            }

            Texts.Text {
                id: itemText

                anchors{
                    left:leftIcon.right
                    leftMargin: 12
                    verticalCenter: parent.verticalCenter
                }
                text: modelData
                color: ColorTheme.textPrimary
                verticalAlignment: Text.AlignVCenter
                font.bold:  root.currentIndex === index
                lineHeight: 20
                lineHeightMode: Text.FixedHeight
            }
        }
    }

    indicator: Buttons.IconButton {
        id: indicatorButton

        anchors{
            right: parent.right
            rightMargin: 12
            verticalCenter: parent.verticalCenter
        }
        icons.source: Images.chevronDown
        sizes: Buttons.Sizes {
            borderLess: true
            horizontalPadding: 4
            verticalPadding: 4
            focusBorderWidth: 0
        }
        colors.background: "transparent"
        colors.focus: "transparent"
        onClicked: {
            root.popup.open();
        }
    }

    popup: Qml.Popup {
        id: popup

        y: root.y + root.sizes.focusBorderWidth
        width: popupWidth === -1 ? root.width + 16 : popupWidth
        height: root.count * itemHeight + root.sizes.popupPadding *2
        implicitHeight: contentItem.implicitHeight
        padding: root.sizes.popupPadding

        contentItem: ListView {

            implicitHeight: contentHeight
            clip: true
            interactive: false
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
        }

        background: Rectangle {
            radius: sizes.popupBorderRadius
            color: ColorTheme.pageBackground
            border{
                width: sizes.popupBorderWidth
                color: ColorTheme.buttonOutline
            }
        }
    }
    Component.onCompleted: {
        contentItem.color = Qt.binding(function() {
            return root.enabled ? ColorTheme.textPrimary : ColorTheme.textDisabled
        })
    }
}


