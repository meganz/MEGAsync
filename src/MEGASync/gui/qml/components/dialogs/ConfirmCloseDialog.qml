import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.checkBoxes 1.0

Window {
    id: root

    readonly property int dialogWidth: 560
    readonly property int dialogHeight: 256
    readonly property int dialogMargin: 48
    readonly property int dialogContentHeight: 100
    readonly property int dialogContentSpacing: 8

    property bool enableBusyIndicator: true
    property alias titleText: title.text
    property alias bodyText: body.text
    property alias cancelButtonText: cancelButton.text
    property alias acceptButtonText: acceptButton.text
    property alias acceptButtonColors: acceptButton.colors
    property alias dontAskAgainVisible: dontAskAgainCB.visible
    property alias dontAskAgainText: dontAskAgainCB.text
    property alias dontAskAgain: dontAskAgainCB.checked

    signal accepted

    width: root.dialogWidth
    height: root.dialogHeight
    minimumWidth: root.dialogWidth
    minimumHeight: root.dialogHeight
    flags: Qt.Dialog
    modality: Qt.WindowModal
    color: colorStyle.surface1
    title: Constants.mega

    Column {
        id: mainColumn

        anchors {
            fill: parent
            margins: dialogMargin
        }
        spacing: dialogMargin / 2 - cancelButton.sizes.focusBorderWidth

        RowLayout {
            id: rowLayout

            width: mainColumn.width
            height: dialogContentHeight
            spacing: mainColumn.spacing

            Image {
                id: leftImage

                source: Images.warning
                sourceSize: Qt.size(dialogContentHeight, dialogContentHeight)
            }

            ColumnLayout {
                id: contentLayout

                spacing: dialogContentSpacing

                Texts.Text {
                    id: title

                    lineHeightMode: Text.FixedHeight
                    lineHeight: 24
                    font {
                        pixelSize: Texts.Text.Size.MEDIUM_LARGE
                        weight: Font.DemiBold
                    }
                }

                Texts.Text {
                    id: body

                    Layout.preferredWidth: 340
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 18
                    font.pixelSize: Texts.Text.Size.NORMAL
                }
            }

        } // RowLayout: rowLayout

        Item {
            id: buttonsLayout

            anchors {
                right: mainColumn.right
                rightMargin: -cancelButton.sizes.focusBorderWidth
                left: mainColumn.left
            }
            height: acceptButton.height + 2 * acceptButton.sizes.focusBorderWidth

            CheckBox {
                id: dontAskAgainCB

                anchors{
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
                implicitWidth: 16

                textWordWrap: Text.NoWrap
                visible: false
            }

            OutlineButton {
                id: cancelButton

                anchors{
                    right: acceptButton.left
                    verticalCenter: parent.verticalCenter
                }
                implicitWidth: 16
                onClicked: {
                    root.close();
                }
            }

            PrimaryButton {
                id: acceptButton


                anchors{
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                onClicked: {
                    if(enableBusyIndicator)
                    {
                        icons.busyIndicatorVisible = true;
                    }
                    root.accepted();
                }
            }
        }

    } // Column: mainColumn

}
