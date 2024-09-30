import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

Qml.MenuItem {
    id: root

    readonly property int itemContentHorizontalPadding: 16
    property Colors colors: Colors {}
    property Sizes sizes: Sizes {
        verticalPadding: 0
    }

    function getBackgroundColor(){
        if(root.pressed) {
            return colors.itemBackgroundPressed;
        }
        else if(root.hovered) {
            return colors.itemBackgroundHover;
        }
        return colors.itemBackground;
    }

    width: parent.width
    height: sizes.itemHeight
    leftPadding: sizes.horizontalPadding
    rightPadding: sizes.horizontalPadding
    topPadding: sizes.verticalPadding
    bottomPadding: sizes.verticalPadding

    contentItem: Rectangle {
        id: itemBorder

        implicitWidth: parent.width
        implicitHeight: parent.height
        color: getBackgroundColor();
        border.color: root.activeFocus ? colors.focus : "transparent";
        border.width: sizes.focusBorderWidth
        radius: sizes.focusRadius

        Row {
            id: row

            anchors.fill: parent
            spacing: sizes.itemContentSpacing
            leftPadding: itemContentHorizontalPadding
            rightPadding: itemContentHorizontalPadding

            SvgImage {
                id: itemImage

                anchors.verticalCenter: parent.verticalCenter
                source: root.icon.source
                sourceSize: Qt.size(sizes.iconWidth, sizes.iconHeight)
                color: colors.icon
            }

            Texts.Text {
                id: itemText

                anchors.verticalCenter: parent.verticalCenter
                verticalAlignment: Text.AlignVCenter
                text: root.text
                font.pixelSize: sizes.textFontSize
                color: colors.text
            }
        }
    }

    background: Rectangle {
        id: itemBackground

        color: "transparent"
    }

    MouseArea {
        id: mouseArea

        anchors.fill: root
        cursorShape: Qt.PointingHandCursor
        onPressed: mouse.accepted = false;
    }
}
