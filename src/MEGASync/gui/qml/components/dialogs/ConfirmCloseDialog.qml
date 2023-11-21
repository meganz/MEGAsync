import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0
import components.buttons 1.0

import onboard 1.0

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
    title: OnboardingStrings.mega

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

                Text {
                    id: title

                    font.pixelSize: Text.Size.MediumLarge
                    font.weight: Font.DemiBold
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 24
                }

                Text {
                    id: body

                    font.pixelSize: Text.Size.Normal
                    Layout.preferredWidth: 340
                    lineHeightMode: Text.FixedHeight
                    lineHeight: 18
                }

            }
        }

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
    }
}
