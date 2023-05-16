// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components 1.0 as Custom

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

            Custom.SvgImage {
                color: button.checked || button.hovered
                       ? Styles.iconAccent
                       : Styles.iconSecondary
                source: imageSource
                sourceSize: imageSourceSize
            }

            Custom.Text {
                text: title
                height: main.textTopHeight
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: main.textHorizontalMargin
                anchors.rightMargin: main.textHorizontalMargin
                font.pixelSize: Custom.Text.Size.MediumLarge
                font.weight: Font.Bold
            }

            Custom.Text {
                text: description
                height: main.textBottomHeight
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: main.textHorizontalMargin
                anchors.rightMargin: main.textHorizontalMargin
                font.pixelSize: Custom.Text.Size.Small
                font.weight: Font.Light
            }
        }
    }
}
