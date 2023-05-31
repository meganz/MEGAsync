// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

CardButton {
    id: button

    contentComponent: Component {

        Column {
            id: main

            readonly property int textSpacing: 8
            readonly property int textTopHeight: 24
            readonly property int textBottomHeight: 64
            readonly property int textHorizontalMargin: 8

            anchors.fill: parent
            anchors.margins: 16
            spacing: 8

            MegaImages.SvgImage {
//                color: button.checked || button.hovered
//                       ? Styles.iconAccent
//                       : Styles.iconSecondary
                source: imageSource
                sourceSize: imageSourceSize
            }

            MegaTexts.Text {
                text: title
                height: main.textTopHeight
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: main.textHorizontalMargin
                anchors.rightMargin: main.textHorizontalMargin
                font.pixelSize: MegaTexts.Text.Size.MediumLarge
                font.weight: Font.Bold
            }

            MegaTexts.Text {
                text: description
                height: main.textBottomHeight
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: main.textHorizontalMargin
                anchors.rightMargin: main.textHorizontalMargin
                font.pixelSize: MegaTexts.Text.Size.Small
                font.weight: Font.Light
                color: Styles.textSecondary
            }
        }
    }
}
