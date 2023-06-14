// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

Rectangle {
    id: root

    // Alias
    property alias textField: textField
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText
    property alias rightIconMouseArea: rightIconMouseArea

    // Component properties
    property bool error: false
    property string title: ""
    property RightIcon rightIcon: RightIcon {}
    property LeftIcon leftIcon: LeftIcon {}
    property Hint hint: Hint {}

    signal backPressed()
    signal pastePressed()

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
            leftMargin: textField.focusBorderWidth
            right: parent.right
            top: parent.top
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
        readonly property int textFieldRawWidth: textField.width - 2 * textField.focusBorderWidth
        readonly property int iconMargin: 13
        readonly property int iconWidth: 16
        readonly property size iconSize: Qt.size(iconWidth, iconWidth)
        readonly property int iconTextSeparation: 6
        readonly property int verticalPadding: 8

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: titleLoader.bottom
        anchors.topMargin: 4

        selectByMouse: true
        selectionColor: Styles.focus
        height: 36 + 2 * focusBorderWidth
        leftPadding: calculatePaddingWithIcon(leftIcon.source != "")
        rightPadding: calculatePaddingWithIcon(rightIcon.source != "")
        topPadding: verticalPadding
        bottomPadding: verticalPadding
        placeholderTextColor: Styles.textPlaceholder
        color: enabled ? Styles.textPrimary : Styles.textDisabled

        font {
            pixelSize: 14
            weight: Font.Light
            family: "Inter"
            styleName: "Medium"
        }

        background: Rectangle {
            id: focusBorder

            color: "transparent"
            border.color: textField.focus ? Styles.focus : "transparent"
            border.width: textField.focusBorderWidth
            radius: textField.focusBorderRadius

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
                anchors.topMargin: textField.iconMargin
                anchors.leftMargin: textField.iconMargin
                z: 2
            }

            Rectangle {

                function getBorderColor() {
                    var color = Styles.borderStrong;
                    if(!enabled) {
                        color = Styles.borderDisabled;
                    } else if(error) {
                        color = Styles.textError;
                    } else if(textField.focus) {
                        color = Styles.borderStrongSelected;
                    }
                    return color;
                }

                anchors {
                    top: focusBorder.top
                    left: focusBorder.left
                    right: focusBorder.right
                    rightMargin: textField.focusBorderWidth
                    topMargin: textField.focusBorderWidth
                    leftMargin: textField.focusBorderWidth
                }
                width: textField.width - 2 * textField.focusBorderWidth
                height: textField.height - 2 * textField.focusBorderWidth
                color: Styles.pageBackground
                border.color: getBorderColor()
                border.width: textField.borderWidth
                radius: textField.borderRadius
            }

            Loader {
                id: rightIconLoader

                anchors.top: focusBorder.top
                anchors.right: focusBorder.right
                anchors.topMargin: textField.iconMargin
                anchors.rightMargin: textField.iconMargin
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
            }
        }
    }

    Loader {
        id: hintLoader

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: textField.bottom
        anchors.topMargin: 2
        anchors.leftMargin: textField.focusBorderWidth
        anchors.rightMargin: textField.focusBorderWidth
    }

    Component {
        id: titleComponent

        MegaTexts.Text {
            id: titleText

            text: title
            font.weight: Font.DemiBold
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
        }
    }

    Component {
        id: leftIconComponent

        MegaImages.SvgImage {
            visible: leftIcon.visible
            source: leftIcon.source
            color: leftIcon.color
            sourceSize: textField.iconSize
            z: 2
        }
    }

    Component {
        id: rightIconComponent

        MegaImages.SvgImage {
            visible: rightIcon.visible
            source: rightIcon.source
            color: rightIcon.color
            sourceSize: textField.iconSize
            z: 2
        }
    }
}






