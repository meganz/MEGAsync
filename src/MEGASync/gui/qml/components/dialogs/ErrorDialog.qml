import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.checkBoxes 1.0

Window {
    id: root

    readonly property int dialogWidth: 600
    readonly property int dialogHeight: 184
    readonly property int dialogMargin: 24
    readonly property int iconSize: 96
    property alias acceptButtonText: acceptButton.text
    property alias titleText: title.text
    property alias bodyText: body.text

    signal accepted

    width: root.dialogWidth
    height: root.dialogHeight
    minimumWidth: root.dialogWidth
    minimumHeight: root.dialogHeight
    maximumWidth: root.dialogWidth
    maximumHeight: root.dialogHeight
    flags: Qt.Dialog
    modality: Qt.WindowModal
    color: ColorTheme.surface1
    title: Constants.mega

    Item {
        id: mainColumn

        anchors {
            fill: parent
            margins: dialogMargin
        }

        Image {
            id: leftImage

            anchors {
                left: parent.left
                top: parent.top
            }
            source: Images.alertTrianglePng
            sourceSize: Qt.size(iconSize, iconSize)
        }

        Texts.RichText {
            id: title

            anchors {
                left: leftImage.right
                leftMargin: dialogMargin
                right: parent.right
                rightMargin: dialogMargin
                top: parent.top
            }

            lineHeightMode: Text.FixedHeight
            lineHeight: 24
            wrapMode: Text.Wrap
            font {
                pixelSize: Texts.Text.Size.MEDIUM_LARGE
                weight: Font.DemiBold
            }
        }

        Texts.RichText {
            id: body

            anchors {
                left: title.left
                top: title.bottom
                topMargin: 4
            }

            width: parent.width
            lineHeightMode: Text.FixedHeight
            lineHeight: 18
            font.pixelSize: Texts.Text.Size.NORMAL
        }

        PrimaryButton {
            id: acceptButton

            anchors{
                right: parent.right
                bottom: parent.bottom
            }

            onClicked: {
                root.close();
            }
        }
    } // Column: mainColumn

}
