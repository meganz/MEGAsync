// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Common 1.0
import Components 1.0 as Custom

Rectangle {
    id: root

    enum Type {
        None = 0,
        Error
    }

    // Component properties
    property int type: TextField.Type.None
    property bool showType: false

    readonly property int focusWidth: textField.focusBorderWidth + textField.borderWidth

    // Title properties
    property string title: ""

    // Left icon textField properties
    property bool leftIconVisible: false
    property url leftIconSource: ""
    property color leftIconColor: Styles.iconSecondary

    // TextField properties
    property alias textField: textField
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText

    // Right icon textField properties
    property bool rightIconVisible: false
    property url rightIconSource: ""
    property color rightIconColor: Styles.iconSecondary
    property alias rightIconMouseArea: rightIconMouseArea

    // Hint properties
    property int hintType: Custom.HintText.Type.None
    property bool hintVisible: false
    property url hintIconSource: ""
    property color hintIconColor
    property string hintTitle: ""
    property color hintTitleColor
    property string hintText: ""
    property color hintTextColor

    signal backPressed()
    signal pastePressed()

    height: textField.height + titleLoader.height + hintLoader.height
    color: "transparent"

    onTitleChanged: {
        if(title.length === 0) {
            return;
        }

        titleLoader.sourceComponent = titleComponent;
    }

    onLeftIconSourceChanged: {
        if(leftIconSource === "") {
            return;
        }

        leftIconVisible = true;
        leftIconLoader.sourceComponent = leftIconComponent;
    }

    onRightIconSourceChanged: {
        if(rightIconSource === "") {
            return;
        }

        rightIconVisible = true;
        rightIconLoader.sourceComponent = rightIconComponent;
    }

    onHintVisibleChanged: {
        if(!hintVisible) {
            return;
        }

        hintLoader.sourceComponent = hintComponent;
    }

    Loader {
        id: titleLoader

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: textField.focusBorderWidth
        anchors.rightMargin: textField.focusBorderWidth
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
        selectionColor: Styles.supportInfo
        height: 42
        leftPadding: calculatePaddingWithIcon(leftIconSource != "")
        rightPadding: calculatePaddingWithIcon(rightIconSource != "")
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
                    var color = Styles.borderDisabled;
                    if(showType && root.type === TextField.Type.Error) {
                        color = Styles.textError;
                    } else if(textField.focus) {
                        color = Styles.borderStrongSelected;
                    } else if(textField.text.length !== 0 && !textField.focus) {
                        color = Styles.borderStrong;
                    }
                    return color;
                }

                anchors.top: focusBorder.top
                anchors.left: focusBorder.left
                anchors.topMargin: textField.focusBorderWidth
                anchors.leftMargin: textField.focusBorderWidth
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

                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
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

        Text {
            id: titleText

            text: title
            color: root.enabled ? Styles.textPrimary : Styles.textDisabled
            font {
                pixelSize: 12
                weight: Font.DemiBold
                family: "Inter"
                styleName: "Medium"
            }
        }
    }

    Component {
        id: hintComponent

        Custom.HintText {
            id: hint

            iconSource: root.hintIconSource
            iconColor: root.hintIconColor
            title: root.hintTitle
            titleColor: root.hintTitleColor
            text: root.hintText
            textColor: root.hintTextColor
            visible: root.hintVisible
            type: root.hintType
        }
    }

    Component {
        id: leftIconComponent

        Custom.SvgImage {
            visible: leftIconVisible
            source: leftIconSource
            color: leftIconColor
            sourceSize: textField.iconSize
            z: 2
        }
    }

    Component {
        id: rightIconComponent

        Custom.SvgImage {
            visible: rightIconVisible
            source: rightIconSource
            color: rightIconColor
            sourceSize: textField.iconSize
            z: 2
        }
    }
}






