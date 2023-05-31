// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

CardButton {
    id: button

    Layout.preferredWidth: 400
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

            MegaImages.SvgImage {
                color: Styles.buttonOutlinePressed /*button.checked || button.hovered
                       ? Styles.iconAccent
                       : Styles.iconSecondary*/
                source: imageSource
                sourceSize: imageSourceSize
                anchors.verticalCenter: parent.verticalCenter
            }

            Column {
                width: parent.width - x
                spacing: main.textSpacing

                MegaTexts.Text {
                    text: title
                    height: main.textTopHeight
                    lineHeightMode: Text.FixedHeight
                    lineHeight: titleLineHeight
                    anchors.left: parent.left
                    anchors.right: parent.right
                    font.pixelSize: MegaTexts.Text.Size.MediumLarge
                    font.weight: Font.Bold
                }

                MegaTexts.Text {
                    text: description
                    height: main.textBottomHeight
                    anchors.left: parent.left
                    anchors.right: parent.right
                    color: Styles.textSecondary
                    font.pixelSize: MegaTexts.Text.Size.Small
                    font.weight: Font.Light
                    lineHeightMode: Text.FixedHeight
                    lineHeight: descriptionLineHeight
                    width: 314
                }
            }
        }
    }
}
