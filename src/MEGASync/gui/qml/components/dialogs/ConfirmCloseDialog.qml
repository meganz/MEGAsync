import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0

Window {
    id: root

    readonly property int dialogWidth: 560
    readonly property int dialogHeight: 256
    readonly property int dialogMargin: 48
    readonly property int dialogContentHeight: 100
    readonly property int dialogContentSpacing: 8

    property alias titleText: title.text
    property alias bodyText: body.text
    property alias cancelButtonText: cancelButton.text
    property alias acceptButtonText: acceptButton.text

    signal accepted

    width: root.dialogWidth
    height: root.dialogHeight
    minimumWidth: root.dialogWidth
    minimumHeight: root.dialogHeight
    flags: Qt.Dialog
    modality: Qt.WindowModal
    color: Styles.surface1
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

        RowLayout {
            id: buttonsLayout

            anchors {
                right: mainColumn.right
                rightMargin: -cancelButton.sizes.focusBorderWidth
            }
            spacing: 0

            OutlineButton {
                id: cancelButton

                onClicked: {
                    root.close();
                }
            }

            PrimaryButton {
                id: acceptButton

                onClicked: {
                    icons.busyIndicatorVisible = true;
                    root.accepted();
                }
            }
        }

    } // Column: mainColumn

}
