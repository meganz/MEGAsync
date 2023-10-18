// System
import QtQuick 2.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Buttons 1.0 as MegaButtons
import Components.ProgressBars 1.0 as MegaProgressBars

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

        MegaProgressBars.HorizontalProgressBar {
            id: progressBar
            value: root.progressValue
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: title.visible && description.visible ? 12 : 0

            MegaTexts.Text {
                id: title
                font.pixelSize: MegaTexts.Text.Size.MediumLarge
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            MegaTexts.RichText {
                id: description
                color: Styles.textSecondary;
                manageMouse: true
                font.pixelSize: MegaTexts.Text.Size.Small
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            MegaButtons.OutlineButton {
                id: leftButton
                onClicked: {
                    guestWindow.hide();
                }
            }

            MegaButtons.PrimaryButton {
                id: rightButton
                onClicked: {
                    guestWindow.hide();
                }
            }
        }
    }
}
