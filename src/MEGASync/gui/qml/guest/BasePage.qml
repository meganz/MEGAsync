import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.progressBars 1.0

Item {
    id: root

    property alias imageSource: image.source
    property alias leftButton: leftButton
    property alias rightButton: rightButton
    property alias showProgressBar: progressBar.visible
    property alias indeterminate: progressBar.indeterminate
    property alias title: title.text
    property alias description: description.rawText
    property alias descriptionUrl: description.url
    property alias descriptionFontSize: description.font.pixelSize
    property alias descriptionColor: description.color
    property alias descriptionLineHeight: description.lineHeight

    property double imageTopMargin: 72
    property double progressValue: 15.0
    property int spacing: 24
    property int bottomMargin: 48
    property int horizontalMargin: 24

    Image {
        id: image

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: imageTopMargin
        source: Images.guest
    }

    Column {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: horizontalMargin
            rightMargin: horizontalMargin
            bottomMargin: bottomMargin
        }
        spacing: root.spacing

        HorizontalProgressBar {
            id: progressBar

            anchors.left: parent.left
            anchors.right: parent.right
            value: root.progressValue
        }

        Column {
            id: textColumn

            anchors.left: parent.left
            anchors.right: parent.right
            spacing: title.visible && description.visible ? 12 : 0

            Texts.Text {
                id: title

                anchors.left: parent.left
                anchors.right: parent.right
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: Texts.Text.Size.MediumLarge
                font.weight: Font.DemiBold
                lineHeight: 24
                lineHeightMode: Text.FixedHeight
                visible: title.text !== ""
            }

            Texts.RichText {
                id: description

                anchors.left: parent.left
                anchors.right: parent.right
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: Styles.textSecondary
                manageMouse: true
                font.pixelSize: Texts.Text.Size.Small
                lineHeight: 16
                lineHeightMode: Text.FixedHeight
                visible: description.text !== ""
            }
        }

        Row {
            id: buttonsRow

            anchors.horizontalCenter: parent.horizontalCenter

            OutlineButton {
                id: leftButton

                onClicked: {
                    window.hide();
                }
            }

            PrimaryButton {
                id: rightButton

                onClicked: {
                    window.hide();
                }
            }
        }
    }
}
