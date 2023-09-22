// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Components.ToolTips 1.0 as MegaToolTips

Rectangle {
    id: root

    // Alias
    property alias textField: textField
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText
    property alias rightIconMouseArea: rightIconMouseArea
    property alias toolTip: toolTip
    property alias acceptableInput: textField.acceptableInput
    property alias validator: textField.validator

    // Component properties
    property bool error: false
    property string title: ""
    property RightIcon rightIcon: RightIcon {}
    property LeftIcon leftIcon: LeftIcon {}
    property Hint hint: Hint {}
    property Sizes sizes: Sizes {}
    property Colors colors: Colors {}

    signal backPressed()
    signal pastePressed()
    signal returnPressed()
    signal accepted()

    function getHintHeight() {
        if(hintLoader.height > 0) {
            return hintLoader.height + hintLoader.anchors.topMargin;
        }
        return hintLoader.height;
    }

    function getTitleHeight() {
        if(titleLoader.height > 0) {
            return titleLoader.height + textField.anchors.topMargin;
        }
        return titleLoader.height;
    }

    Layout.preferredHeight: height
    height: textField.height + getTitleHeight() + getHintHeight()
    color: "transparent"

    onTitleChanged: {
        if(title.length === 0) {
            return;
        }

        titleLoader.sourceComponent = titleComponent;
    }

    Loader {
        id: titleLoader

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            leftMargin: sizes.focusBorderWidth
            bottomMargin: sizes.titleBottomMargin
        }
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
        anchors.top: titleLoader.bottom
        anchors.topMargin: sizes.titleSpacing

        selectByMouse: true
        selectionColor: colors.selection
        height: sizes.height + 2 * sizes.focusBorderWidth
        leftPadding: calculatePaddingWithIcon(leftIcon.source != "")
        rightPadding: calculatePaddingWithIcon(rightIcon.source != "")
        topPadding: sizes.padding
        bottomPadding: sizes.padding
        placeholderTextColor: colors.placeholder
        color: enabled ? colors.text : colors.textDisabled

        onAccepted: {
            root.accepted()
        }

        font {
            pixelSize: MegaTexts.Text.Medium
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

            Loader {
                id: leftIconLoader

                anchors.top: focusBorder.top
                anchors.left: focusBorder.left
                anchors.topMargin: sizes.iconMargin
                anchors.leftMargin: sizes.iconMargin
                z: 2
            }

            Rectangle {

                function getBorderColor() {
                    var color = colors.border;
                    if(!enabled) {
                        color = colors.borderDisabled;
                    } else if(error) {
                        color = colors.borderError;
                    } else if(textField.focus) {
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

            Loader {
                id: rightIconLoader

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

    Loader {
        id: hintLoader

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: textField.bottom
        anchors.topMargin: 2
        anchors.leftMargin: sizes.focusBorderWidth
        anchors.rightMargin: sizes.focusBorderWidth
    }

    Component {
        id: titleComponent

        MegaTexts.Text {
            id: titleText

            text: title
            font.weight: Font.DemiBold
            color: enabled ? colors.title : colors.titleDisabled
        }
    }

    Component {
        id: hintComponent

        MegaTexts.HintText {
            id: hint

            icon: root.hint.icon
            title: root.hint.title
            text: root.hint.text
            styles: root.hint.styles
            visible: root.hint.visible
            textSize: root.sizes.hintTextSize
        }
    }

    Component {
        id: leftIconComponent

        MegaImages.SvgImage {
            visible: leftIcon.visible
            source: leftIcon.source
            color: leftIcon.color
            sourceSize: sizes.iconSize
            z: 2
        }
    }

    Component {
        id: rightIconComponent

        MegaImages.SvgImage {
            visible: rightIcon.visible
            source: rightIcon.source
            color: rightIcon.color
            sourceSize: sizes.iconSize
            z: 2
        }
    }

    MegaToolTips.ToolTip {
        id: toolTip

        visible: textField.text
                    && textField.readOnly
                    && textField.contentWidth > textField.width - textField.leftPadding - textField.rightPadding
                    && textField.hovered
        text: textField.text
    }

}
