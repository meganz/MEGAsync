import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

Item {
    id: root

    property alias text: textItem.text
    property alias source: iconItem.source

    property Colors colors: Colors {}
    property MediumSizes sizes: MediumSizes {}

    width: container.width
    height: container.height

    Rectangle {
        id: container

        property real horizontalPadding: text.length === 0 ? sizes.verticalPadding : sizes.horizontalPadding

        anchors.centerIn: parent
        radius: sizes.radius
        color: colors.background
        border {
            color: colors.border
            width: sizes.borderWidth
        }
        width: content.width + 2 * container.horizontalPadding
        height: content.height + 2 * sizes.verticalPadding

        Row {
            id: content

            anchors {
                centerIn: parent
                leftMargin: container.horizontalPadding
                rightMargin: container.horizontalPadding
                topMargin: sizes.verticalPadding
                bottomMargin: sizes.verticalPadding
            }
            spacing: sizes.spacing

            SvgImage {
                id: iconItem

                anchors.verticalCenter: parent.verticalCenter
                sourceSize: sizes.iconSize
                color: colors.icon
            }

            Texts.Text {
                id: textItem

                color: colors.text
                font {
                    pixelSize: sizes.pixelSize
                    weight: sizes.fontWeight
                }
                lineHeightMode: Text.FixedHeight
                lineHeight: sizes.lineHeight
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
