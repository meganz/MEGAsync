import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.toolTips 1.0

FocusScope {
    id: root

    property alias textField: textField
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText
    property alias rightIconMouseArea: rightIconMouseArea
    property alias toolTip: toolTip
    property alias acceptableInput: textField.acceptableInput
    property alias validator: textField.validator
    property alias hint: hintItem
    property alias title: titleItem.text
    property alias rightIconVisible: rightIcon.visible
    property alias leftIconColor: leftIcon.color
    property alias leftIconVisible: leftIcon.visible

    property bool error: false
    property string rightIconSource: ""
    property string leftIconSource: ""

    property Sizes sizes: Sizes {}
    property Colors colors: Colors {}

    signal backPressed
    signal pastePressed
    signal returnPressed
    signal accepted

    height: textField.height
                + ((titleItem.text !== "" && titleItem.visible)
                    ? (titleItem.height + textField.anchors.topMargin)
                    : 0)
                + ((hintItem.text !== "" && hintItem.visible)
                    ? hint.height + hint.anchors.topMargin
                    : 0)
    Layout.preferredHeight: height

    onLeftIconSourceChanged: {
        if (leftIconSource.length > 0) {
            leftIcon.source = leftIconSource
        }
    }

    onRightIconSourceChanged: {
        if (rightIconSource.length > 0) {
            rightIcon.source = rightIconSource
        }
    }

    Texts.Text {
        id: titleItem

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            leftMargin: sizes.focusBorderWidth
            bottomMargin: sizes.titleBottomMargin
        }
        font.weight: Font.DemiBold
        color: enabled ? colors.title : colors.titleDisabled
    }

    Qml.TextField {
        id: textField

        function calculatePaddingWithIcon(iconPresent) {
            var padding = sizes.iconMargin;
            if(iconPresent) {
                padding += sizes.iconWidth + sizes.iconTextSeparation;
            }
            else {
                padding += sizes.focusBorderWidth;
            }
            return padding;
        }

        anchors {
            left: parent.left
            right: parent.right
            top: titleItem.text.length > 0 ? titleItem.bottom : parent.top
            topMargin: sizes.titleSpacing
        }

        selectByMouse: true
        selectionColor: colors.selection
        focus: true
        height: sizes.height + 2 * sizes.focusBorderWidth
        leftPadding: calculatePaddingWithIcon(leftIconSource != "")
        rightPadding: calculatePaddingWithIcon(rightIconSource != "")
        topPadding: sizes.padding
        bottomPadding: sizes.padding
        placeholderTextColor: colors.placeholder
        color: enabled ? colors.text : colors.textDisabled
        font {
            pixelSize: Texts.Text.Size.MEDIUM
            family: Styles.fontFamily
            styleName: Styles.fontStyleName
        }

        onAccepted: {
            root.accepted();
        }

        background: Rectangle {
            id: focusBorder

            anchors {
                left: textField.left
                right: textField.right
                top: textField.top
                bottom: textField.bottom
            }
            radius: sizes.focusBorderRadius
            color: "transparent"
            border {
                color: root.activeFocus || textField.activeFocus ? colors.focus : "transparent"
                width: sizes.focusBorderWidth
            }

            SvgImage {
                id: leftIcon

                anchors {
                    top: focusBorder.top
                    left: focusBorder.left
                    topMargin: sizes.iconMargin
                    leftMargin: sizes.iconMargin
                }
                color: enabled ? colors.icon : colors.iconDisabled
                sourceSize: sizes.iconSize
                z: 2
            }

            Rectangle {
                id: backgroundRect

                function getBorderColor() {
                    var color = colors.border;
                    if(!enabled) {
                        color = colors.borderDisabled;
                    }
                    else if(error) {
                        color = colors.borderError;
                    }
                    else if(root.activeFocus || textField.activeFocus) {
                        color = colors.borderFocus;
                    }
                    return color;
                }

                anchors {
                    top: focusBorder.top
                    left: focusBorder.left
                    right: focusBorder.right
                    rightMargin: sizes.focusBorderWidth
                    topMargin: sizes.focusBorderWidth
                    leftMargin: sizes.focusBorderWidth
                }

                width: textField.width - 2 * sizes.focusBorderWidth
                height: textField.height - 2 * sizes.focusBorderWidth
                radius: sizes.borderRadius
                color: colors.background
                border {
                    color: getBorderColor()
                    width: sizes.borderWidth
                }
            }

            SvgImage {
                id: rightIcon

                anchors {
                    top: focusBorder.top
                    right: focusBorder.right
                    topMargin: sizes.iconMargin
                    rightMargin: sizes.iconMargin
                }
                sourceSize: sizes.iconSize
                color: enabled ? colors.icon : colors.iconDisabled
                z: 2

                MouseArea {
                    id: rightIconMouseArea

                    anchors.fill: parent
                    enabled: rightIcon.visible
                    cursorShape: rightIcon.visible ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

        } // Rectangle: focusBorder

        Keys.onPressed: {
            if(event.key === Qt.Key_Backspace) {
                root.backPressed();
            }
            else if((event.key === Qt.Key_V) && (event.modifiers & Qt.ControlModifier)) {
                pastePressed();
            }
            else if(event.key === Qt.Key_Up) {
                textField.cursorPosition = 0;
            }
            else if(event.key === Qt.Key_Down) {
                textField.cursorPosition = textField.text.length;
            }
            else if(event.key === Qt.Key_Return) {
                root.returnPressed();
            }
        }

    } // Qml.TextField: textField

    Texts.HintText {
        id: hintItem

        anchors {
            left: parent.left
            right: parent.right
            top: textField.bottom
            topMargin: 2
            leftMargin: sizes.focusBorderWidth
            rightMargin: sizes.focusBorderWidth
        }
        type: Constants.MessageType.ERROR
        visible: false
    }

    ToolTip {
        id: toolTip

        text: textField.text
        visible: textField.text
                    && textField.readOnly
                    && textField.contentWidth
                            > textField.width - textField.leftPadding - textField.rightPadding
                    && textField.hovered
    }
}
