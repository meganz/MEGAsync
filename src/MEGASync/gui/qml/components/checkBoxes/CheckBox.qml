// System
import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

// Local
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

Qml.CheckBox {
    id: root

    function indeterminate() {
        return checkState === Qt.PartiallyChecked;
    }

    property string url: ""
    property Sizes sizes: Sizes {}
    property Colors colors: Colors {}
    property Icons icons: Icons {}

    spacing: (text.length === 0) ? 0 : sizes.spacing
    height: Math.max(contentItem.height, indicator.height)
    padding: 0

    contentItem: MegaTexts.RichText {
        anchors.left: indicator.right
        leftPadding: root.spacing
        height: Math.max(contentItem.implicitHeight, indicator.height)
        rawText: root.text
        wrapMode: Text.WordWrap
        fontSizeMode: Text.Fit
        url: root.url
        verticalAlignment: Text.AlignVCenter

        MouseArea {
            anchors.fill: parent
            onPressed: { mouse.accepted = false; }
            cursorShape: Qt.PointingHandCursor
        }
    }

    indicator: Rectangle {
        id: checkBoxOutRect

        function getBorderColor() {
            var color = colors.border;
            if(!root.enabled) {
                color = colors.borderDisabled;
            } else if(root.pressed) {
                color = colors.borderPressed;
            } else if(root.hovered) {
                color = colors.borderHover;
            }
            return color;
        }

        width: sizes.indicatorWidth
        height: sizes.indicatorWidth
        radius: sizes.indicatorRadius
        border.color: checkBoxOutRect.getBorderColor()
        border.width: sizes.indicatorBorderWidth
        color: "transparent"

        Rectangle {
            id: inside

            function getBackgroundColor() {
                var color = colors.backgroundUnchecked;
                if(checkState === Qt.Unchecked) {
                    return color;
                }

                if(!root.enabled) {
                    color = colors.backgroundDisabled;
                } else if(root.pressed) {
                    color = colors.backgroundPressed;
                } else if(root.hovered) {
                    color = colors.backgroundHover;
                } else {
                    color = colors.background;
                }

                return color;
            }

            visible: root.checked || root.down || indeterminate()
            color: getBackgroundColor()
            radius: 1
            width: checkBoxOutRect.width - checkBoxOutRect.border.width
            height: inside.width
            anchors.centerIn: checkBoxOutRect

            MegaImages.SvgImage {
                id: image

                visible: indeterminate() || checked
                source: indeterminate() ? icons.indeterminate : icons.checked
                anchors.centerIn: inside
                sourceSize: indeterminate() ? sizes.iconSizeIndeterminate : sizes.iconSize
                color: Styles.iconInverseAccent
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onPressed: { mouse.accepted = false; }
        cursorShape: Qt.PointingHandCursor
    }

}
