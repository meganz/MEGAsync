import QtQuick 2.15

import components.images 1.0
import components.texts 1.0 as Texts

Item {
    id: root

    property alias text: richText.text
    property alias font: richText.font
    property alias textColor: richText.color
    property alias backgroundColor: mainRect.color
    property alias iconSource: iconImage.source
    property alias iconSize: iconImage.sourceSize
    property alias radius: mainRect.radius

    property int iconTextSpacing: 4
    property int horizontalPadding: 4
    property int verticalPadding: 2


    implicitWidth: richText.implicitWidth + (iconImage.source.length === 0? 0 : iconImage.width )
                   + 2 * horizontalPadding + iconTextSpacing
    implicitHeight: Math.max(richText.height, iconImage.height) + 2 * verticalPadding

    Rectangle{
        id: mainRect

        implicitWidth:  parent.implicitWidth
        implicitHeight: parent.implicitHeight

        SvgImage {
            id: iconImage

            sourceSize: Qt.size(16, 16)
            anchors{
                left: parent.left
                verticalCenter: parent.verticalCenter
                leftMargin: horizontalPadding
            }
            visible: root.visible
        }

        Texts.RichText {
            id: richText

            visible: root.visible
            wrapMode: Text.NoWrap
            anchors{
                left: iconImage.right
                leftMargin: iconTextSpacing
                verticalCenter: parent.verticalCenter
            }
        }
    }
}
