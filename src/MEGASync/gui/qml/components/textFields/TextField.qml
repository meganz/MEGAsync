// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Common 1.0
import Components 1.0 as Custom

ColumnLayout {
    id: root

    enum DescriptionType {
        None = 0,
        InfoWithoutIcon = 1,
        Info = 2,
        Error = 3
    }

    property alias hint: hint
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText
    property alias textField: textField
    property alias leftIcon: leftIcon
    property alias rightIcon: rightIcon
    property alias rightIconMouseArea: rightIconMouseArea

    property string title: ""

    readonly property int textFieldRawHeight: textField.height - 2 * textField.focusBorderWidth

    signal backPressed()
    signal pastePressed()

    spacing: title.lenght === 0 ? 0 : 4

    Text {
        text: title
        color: Styles.textPrimary
        visible: title.length !== 0
        Layout.leftMargin: 4
        Layout.preferredHeight: 16
        font {
            pixelSize: 12
            weight: Font.Bold
            family: "Inter"
            styleName: "Medium"
        }
    }

    Qml.TextField {
        id: textField

        function calculatePaddingWithIcon(iconPresent) {
            var padding = iconMargin;
            if(iconPresent) {
                padding += iconWidth + iconTextSeparation;
            } else {
                padding += focusBorderWidth;
            }
            return padding;
        }

        readonly property int focusBorderRadius: 11
        readonly property int focusBorderWidth: 3
        readonly property int borderRadius: 8
        readonly property int borderWidth: 1
        readonly property int iconMargin: 13
        readonly property int iconWidth: 16
        readonly property size iconSize: Qt.size(iconWidth, iconWidth)
        readonly property int iconTextSeparation: 6
        readonly property int verticalPadding: 8

        selectByMouse: true
        selectionColor: Styles.supportInfo
        Layout.preferredWidth: parent.width
        height: 42
        Layout.preferredHeight: height
        leftPadding: calculatePaddingWithIcon(leftIcon.source != "")
        rightPadding: calculatePaddingWithIcon(rightIcon.source != "")
        topPadding: verticalPadding
        bottomPadding: verticalPadding

        font {
            pixelSize: 14
            weight: Font.Light
            family: "Inter"
            styleName: "Medium"
        }

        background: Rectangle {
            id: focusBorder

            color: "transparent"
            border.color: textField.focus ? Styles.supportInfo : "transparent"
            border.width: textField.focusBorderWidth
            radius: textField.focusBorderRadius

            Custom.SvgImage {
                id: leftIcon

                visible: leftIcon.source != ""
                sourceSize: textField.iconSize
                color: Styles.iconSecondary
                anchors.top: focusBorder.top
                anchors.left: focusBorder.left
                anchors.topMargin: textField.iconMargin
                anchors.leftMargin: textField.iconMargin
                z: 2
            }

            Rectangle {

                function getBorderColor() {
                    var color = Styles.borderDisabled;
                    if(hint.type === Custom.HintText.Type.Error) {
                        color = Styles.textError;
                    } else if(textField.focus) {
                        color = Styles.borderStrongSelected;
                    } else if(textField.text.length !== 0 && !textField.focus) {
                        color = Styles.borderStrong;
                    }
                    return color;
                }

                width: textField.width - 2 * textField.focusBorderWidth
                height: textField.height - 2 * textField.focusBorderWidth
                color: Styles.pageBackground
                border.color: getBorderColor()
                border.width: textField.borderWidth
                radius: textField.borderRadius
                anchors.top: focusBorder.top
                anchors.left: focusBorder.left
                anchors.topMargin: textField.focusBorderWidth
                anchors.leftMargin: textField.focusBorderWidth
            }

            Custom.SvgImage {
                id: rightIcon

                sourceSize: textField.iconSize
                color: Styles.iconSecondary
                anchors.top: focusBorder.top
                anchors.right: focusBorder.right
                anchors.topMargin: textField.iconMargin
                anchors.rightMargin: textField.iconMargin
                z: 2

                MouseArea {
                    id: rightIconMouseArea

                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        Keys.onPressed:
        {
            if(event.key === Qt.Key_Backspace)
            {
                root.backPressed();
            }
            else if((event.key === Qt.Key_V) && (event.modifiers & Qt.ControlModifier)) {
                pastePressed();
            }
        }
    }

    Custom.HintText {
        id: hint

        Layout.fillWidth: true
        Layout.leftMargin: 4
        Layout.preferredHeight: hint.height
    }
}






