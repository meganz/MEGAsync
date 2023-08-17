// System
import QtQuick 2.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Buttons 1.0 as MegaButtons
import Components.ProgressBars 1.0 as MegaProgressBars

Item {
    id: basePage

    property alias image: image
    property alias leftButton: leftButton
    property alias rightButton: rightButton

    property double imageTopMargin: 72
    property bool showProgressBar: false
    property bool indeterminate: false
    property double progressValue: 15.0
    property string title: ""
    property string description: ""
    property int spacing: 24

    onShowProgressBarChanged: {
        if(!showProgressBar) {
            return;
        }

        progressBarLoader.sourceComponent = progressBar;
    }

    onTitleChanged: {
        if(title === "") {
            return;
        }

        titleLoader.sourceComponent = titleComponent;
    }

    onDescriptionChanged: {
        if(description === "") {
            return;
        }

        descriptionLoader.sourceComponent = descriptionComponent;
    }

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
        anchors.bottomMargin: 48
        spacing: basePage.spacing

        Loader {
            id: progressBarLoader

            anchors.left: parent.left
            anchors.right: parent.right
            visible: showProgressBar
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: titleLoader.visible && descriptionLoader.visible ? 12 : 0

            Loader {
                id: titleLoader

                anchors.left: parent.left
                anchors.right: parent.right
                visible: title !== ""
            }

            Loader {
                id: descriptionLoader

                anchors.left: parent.left
                anchors.right: parent.right
                visible: description !== ""
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

    Component {
        id: progressBar

        MegaProgressBars.HorizontalProgressBar {
            indeterminate: basePage.indeterminate
            value: progressValue
        }
    }

    Component {
        id: titleComponent

        MegaTexts.Text {
            id: titleText

            text: title
            font.pixelSize: MegaTexts.Text.Size.MediumLarge
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Component {
        id: descriptionComponent

        MegaTexts.SecondaryText {
            id: titleText

            text: description
            font.pixelSize: MegaTexts.Text.Size.Small
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
