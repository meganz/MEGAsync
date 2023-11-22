import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

CardButton {
    id: button

    Layout.preferredWidth: 408
    Layout.preferredHeight: 88
    Layout.fillWidth: true
    width: 400
    height: 88
    imageSourceSize: Qt.size(48, 48)

    contentComponent: Component {

        Row {
            id: main

            readonly property int horizontalMargin: 24
            readonly property int verticalMargin: 14
            readonly property int textSpacing: 4
            readonly property int titleLineHeight: 24
            readonly property int descriptionLineHeight: 16

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
                spacing: main.textSpacing

                Texts.Text {
                    text: title
                    height: main.textTopHeight
                    lineHeightMode: Text.FixedHeight
                    lineHeight: titleLineHeight
                    anchors.left: parent.left
                    anchors.right: parent.right
                    font.pixelSize: Texts.Text.Size.MediumLarge
                    font.weight: Font.Bold
                }

                Texts.Text {
                    text: description
                    height: main.textBottomHeight
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
}
