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
    property double imageTopMargin: 72
    property alias showProgressBar: progressBar.visible
    property alias indeterminate: progressBar.indeterminate
    property double progressValue: 15.0
    property alias title: title.text
    property alias description: description.rawText
    property alias descriptionUrl: description.url
    property int spacing: 24
    property int bottomMargin: 48

    Image {
        id: image

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: imageTopMargin
        source: Images.guest
    }

    Column {
        width: 304
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: bottomMargin
        spacing: root.spacing

        HorizontalProgressBar {
            id: progressBar

            anchors.left: parent.left
            anchors.right: parent.right
            value: root.progressValue
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: title.visible && description.visible ? 12 : 0

            Texts.Text {
                id: title

                font.pixelSize: Texts.Text.Size.MediumLarge
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.left: parent.left
                anchors.right: parent.right
            }

            Texts.RichText {
                id: description

                color: Styles.textSecondary;
                manageMouse: true
                font.pixelSize: Texts.Text.Size.Small
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.left: parent.left
                anchors.right: parent.right
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            OutlineButton {
                id: leftButton

                onClicked: {
                    guestWindow.hide();
                }
            }

            PrimaryButton {
                id: rightButton

                onClicked: {
                    guestWindow.hide();
                }
            }
        }
    }
}
