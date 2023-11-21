import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0
import components.images 1.0

CardButton {
    id: button

    property int textHorizontalExtraMargin: 0
    property int contentMargin: 12
    property int contentSpacing: 24

    contentComponent: Component {

        Column {
            id: main

            readonly property int textSpacing: 8
            readonly property int textTopHeight: 24
            readonly property int textTopMargin: 24
            readonly property int textLineHeight: 16

            anchors.fill: parent
            anchors.margins: contentMargin
            spacing: contentSpacing

            SvgImage {
                source: imageSource
                sourceSize: imageSourceSize
            }

            Column {
                spacing: textSpacing
                anchors.left: parent.left
                anchors.right: parent.right

                Text {
                    text: title
                    height: main.textTopHeight
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: main.textTopMargin
                    anchors.leftMargin: button.textHorizontalExtraMargin
                    anchors.rightMargin: button.textHorizontalExtraMargin
                    font.pixelSize: Text.Size.MediumLarge
                    font.weight: Font.Bold
                }

                Text {
                    text: description
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: button.textHorizontalExtraMargin
                    anchors.rightMargin: button.textHorizontalExtraMargin
                    font.pixelSize: Text.Size.Small
                    color: Styles.textSecondary
                    lineHeight: main.textLineHeight
                    lineHeightMode: Text.FixedHeight
                }
            }
        }
    }
}
