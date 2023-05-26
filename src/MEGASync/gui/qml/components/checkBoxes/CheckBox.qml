// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml

// Local
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

Qml.CheckBox {
    id: checkBox

    function indeterminate() {
        return checkState === Qt.PartiallyChecked;
    }

    function toIndeterminate() {
        tristate = true;
        checkState = Qt.PartiallyChecked;
    }

    function fromIndeterminate(value) {
        tristate = false;
        checkState = value ? Qt.Checked : Qt.Unchecked;
    }

    property string url: ""
    property Sizes sizes: Sizes {}
    property Colors colors: Colors {}
    property Icons icons: Icons {}

    spacing: (text.length === 0) ? 0 : 8
    indicator: checkBoxOutRect
    contentItem: Loader { id: textLoader }
    padding: 0
    height: (text.length === 0) ? checkBoxOutRect.height : textLoader.height

    onTextChanged: {
        if(text.length === 0) {
            return;
        }

        textLoader.sourceComponent = textComponent;
    }

    Rectangle {
        id: checkBoxOutRect

        function getBorderColor() {
            var color = colors.border;
            if(!checkBox.enabled) {
                color = colors.borderDisabled;
            } else if(checkBox.pressed) {
                color = colors.borderPressed;
            } else if(checkBox.hovered) {
                color = colors.borderHover;
            }
            return color;
        }

        function getBackgroundColor() {
            var color = colors.backgroundUnchecked;
            if(checkState === Qt.Unchecked) {
                return color;
            }

            if(!checkBox.enabled) {
                color = colors.backgroundDisabled;
            } else if(checkBox.pressed) {
                color = colors.backgroundPressed;
            } else if(checkBox.hovered) {
                color = colors.backgroundHover;
            } else {
                color = colors.background;
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

            visible: checkBox.checked || checkBox.down || indeterminate()
            color: checkBoxOutRect.getBackgroundColor()
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

    Component {
        id: textComponent

        MegaTexts.RichText {
            text: checkBox.text
            leftPadding: checkBoxOutRect.width + checkBox.spacing
            wrapMode: Text.WordWrap
            fontSizeMode: Text.Fit
            url: checkBox.url
            textFormat: Text.AutoText
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: mouse.accepted = false
        cursorShape: Qt.PointingHandCursor
    }

}
