import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

CardButton {
    id: button

    readonly property int horizontalMargin: 24
    readonly property int verticalMargin: 14
    readonly property int textSpacing: 4
    readonly property int titleLineHeight: 24
    readonly property int descriptionLineHeight: 16

    Layout.preferredWidth: 408
    Layout.preferredHeight: 88
    Layout.fillWidth: true
    width: 400
    height: titleText.height + descriptionText.height + textSpacing + verticalMargin * 2
    imageSourceSize: Qt.size(48, 48)

    Row {
        anchors.fill: parent
        anchors.leftMargin: horizontalMargin
        anchors.rightMargin: horizontalMargin
        anchors.topMargin: verticalMargin
        anchors.bottomMargin: verticalMargin
        spacing: 20

        SvgImage {
            source: imageSource
            sourceSize: imageSourceSize
            anchors.verticalCenter: parent.verticalCenter
        }

        Column {
            width: parent.width - x
            spacing: textSpacing

            Texts.Text {
                id: titleText

                text: title
                height: textTopHeight
                lineHeightMode: Text.FixedHeight
                lineHeight: titleLineHeight
                anchors.left: parent.left
                anchors.right: parent.right
                font.pixelSize: Texts.Text.Size.MediumLarge
                font.weight: Font.Bold
            }

            Texts.Text {
                id: descriptionText

                text: description
                height: textBottomHeight
                anchors.left: parent.left
                anchors.right: parent.right
                color: Styles.textSecondary
                font.pixelSize: Texts.Text.Size.Small
                lineHeightMode: Texts.Text.FixedHeight
                lineHeight: descriptionLineHeight
                width: 314
            }
        }
    }
}
