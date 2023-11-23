import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

CardButton {
    id: button

    property int textHorizontalExtraMargin: 0
    property int contentMargin: 12
    property int contentSpacing: 24

    readonly property int textSpacing: 8
    readonly property int textTopHeight: 24
    readonly property int textTopMargin: 24
    readonly property int textLineHeight: 16

    height: imageButton.height + titleText.height + descriptionText.height + textSpacing + contentSpacing + contentMargin * 2

    Column {
        anchors.fill: parent
        anchors.margins: contentMargin
        spacing: contentSpacing

        SvgImage {
            id: imageButton

            source: imageSource
            sourceSize: imageSourceSize
        }

        Column {
            spacing: textSpacing
            anchors.left: parent.left
            anchors.right: parent.right

            Texts.Text {
                id: titleText

                text: title
                height: main.textTopHeight
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: main.textTopMargin
                anchors.leftMargin: button.textHorizontalExtraMargin
                anchors.rightMargin: button.textHorizontalExtraMargin
                font.pixelSize: Texts.Text.Size.MediumLarge
                font.weight: Font.Bold
            }

            Texts.Text {
                id: descriptionText

                text: description
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: button.textHorizontalExtraMargin
                anchors.rightMargin: button.textHorizontalExtraMargin
                font.pixelSize: Texts.Text.Size.Small
                color: Styles.textSecondary
                lineHeight: textLineHeight
                lineHeightMode: Text.FixedHeight
            }
        }
    }
}
