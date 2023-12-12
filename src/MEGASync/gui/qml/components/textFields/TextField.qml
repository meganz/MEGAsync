import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.toolTips 1.0

FocusScope {
    id: root

    // Alias
    property alias textField: textField
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText
    property alias rightIconMouseArea: rightIconMouseArea
    property alias toolTip: toolTip
    property alias acceptableInput: textField.acceptableInput
    property alias validator: textField.validator
    property alias hint: hintItem

    // Component properties
    property bool error: false
    property alias title: titleItem.text

    property alias rightIconVisible: rightIcon.visible
    property string rightIconSource: ""

    property alias leftIconColor: leftIcon.color
    property alias leftIconVisible: leftIcon.visible
    property string leftIconSource: ""

    property Sizes sizes: Sizes {}
    property Colors colors: Colors {}

    signal backPressed()
    signal pastePressed()
    signal returnPressed()
    signal accepted()

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

    Layout.preferredHeight: height
    height: textField.height + ((titleItem.text !== "" && titleItem.visible) ? (titleItem.height + textField.anchors.topMargin) : 0) +
            ((hintItem.text !== "" && hintItem.visible) ? hint.height + hint.anchors.topMargin : 0)

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
            } else {
                padding += sizes.focusBorderWidth;
            }
            return padding;
        }

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: titleItem.text.length > 0 ? titleItem.bottom : parent.top
        anchors.topMargin: sizes.titleSpacing
        focus: true
        selectByMouse: true
        selectionColor: colors.selection
        height: sizes.height + 2 * sizes.focusBorderWidth
        leftPadding: calculatePaddingWithIcon(leftIconSource != "")
        rightPadding: calculatePaddingWithIcon(rightIconSource != "")
        topPadding: sizes.padding
        bottomPadding: sizes.padding
        placeholderTextColor: colors.placeholder
        color: enabled ? colors.text : colors.textDisabled
        onAccepted: {
            root.accepted()
        }

        font {
            pixelSize: Texts.Text.Size.Medium
            family: Styles.fontFamily
            styleName: Styles.fontStyleName
        }

        background: Rectangle {
            id: focusBorder

            color: "transparent"
            border.color: textField.activeFocus ? colors.focus : "transparent"
            border.width: sizes.focusBorderWidth
            radius: sizes.focusBorderRadius

            anchors {
                left: textField.left
                right: textField.right
                top: textField.top
                bottom: textField.bottom
            }

            SvgImage {
                id: leftIcon

                color: enabled ? colors.icon : colors.iconDisabled
                anchors.top: focusBorder.top
                anchors.left: focusBorder.left
                anchors.topMargin: sizes.iconMargin
                anchors.leftMargin: sizes.iconMargin
                sourceSize: sizes.iconSize
                z: 2
            }

            Rectangle {

                function getBorderColor() {
                    var color = colors.border;
                    if(!enabled) {
                        color = colors.borderDisabled;
                    } else if(error) {
                        color = colors.borderError;
                    } else if(root.focus) {
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
                color: colors.background
                border.color: getBorderColor()
                border.width: sizes.borderWidth
                radius: sizes.borderRadius
            }

            SvgImage {
                id: rightIcon

                color: enabled ? colors.icon : colors.iconDisabled
                sourceSize: sizes.iconSize
                anchors.top: focusBorder.top
                anchors.right: focusBorder.right
                anchors.topMargin: sizes.iconMargin
                anchors.rightMargin: sizes.iconMargin
                z: 2

                MouseArea {
                    id: rightIconMouseArea

                    enabled: rightIcon.visible
                    anchors.fill: parent
                    cursorShape: rightIcon.visible ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }
        }

        Keys.onPressed: {
            if(event.key === Qt.Key_Backspace) {
                root.backPressed();
            } else if((event.key === Qt.Key_V) && (event.modifiers & Qt.ControlModifier)) {
                pastePressed();
            } else if(event.key === Qt.Key_Up) {
                textField.cursorPosition = 0;
            } else if(event.key === Qt.Key_Down) {
                textField.cursorPosition = textField.text.length;
            } else if(event.key === Qt.Key_Return) {
                root.returnPressed()
            }
        }
    }

    Texts.HintText {
        id: hintItem

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: textField.bottom
        anchors.topMargin: 2
        anchors.leftMargin: sizes.focusBorderWidth
        anchors.rightMargin: sizes.focusBorderWidth
        type: Constants.MessageType.ERROR
        visible: false
    }

    ToolTip {
        id: toolTip

        visible: textField.text
                    && textField.readOnly
                    && textField.contentWidth > textField.width - textField.leftPadding - textField.rightPadding
                    && textField.hovered
        text: textField.text
    }
}
