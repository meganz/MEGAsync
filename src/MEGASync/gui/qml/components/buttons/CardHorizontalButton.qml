import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

CardButton {
    id: root

    readonly property int horizontalMargin: 24
    readonly property int verticalMargin: 14
    readonly property int textSpacing: 4
    readonly property int titleLineHeight: 24
    readonly property int descriptionLineHeight: 16

    width: 400
    height: titleText.height + descriptionText.height + textSpacing + verticalMargin * 2
    Layout.preferredWidth: 408
    Layout.preferredHeight: height
    Layout.fillWidth: true
    imageSourceSize: Qt.size(48, 48)

    Row {
        id: mainRow

        anchors {
            fill: parent
            leftMargin: horizontalMargin
            rightMargin: horizontalMargin
            topMargin: verticalMargin
            bottomMargin: verticalMargin
        }
        spacing: 20

        SvgImage {
            id: icon

            anchors.verticalCenter: parent.verticalCenter
            source: imageSource
            sourceSize: imageSourceSize
        }

        Column {
            id: textsColumn

            width: parent.width - x
            spacing: textSpacing

            Texts.Text {
                id: titleText

                anchors {
                    left: parent.left
                    right: parent.right
                }
                text: title
                lineHeightMode: Text.FixedHeight
                lineHeight: titleLineHeight
                font {
                    pixelSize: Texts.Text.Size.MEDIUM_LARGE
                    weight: Font.Bold
                }
            }

            Texts.Text {
                id: descriptionText

                anchors {
                    left: parent.left
                    right: parent.right
                }
                width: 314
                text: description
                color: colorStyle.textSecondary
                font.pixelSize: Texts.Text.Size.SMALL
                lineHeightMode: Texts.Text.FixedHeight
                lineHeight: descriptionLineHeight
            }
        }

    } // Row: mainRow

}
