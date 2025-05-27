import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0

Window {
    id: root

    readonly property int dialogWidth: 500
    readonly property int dialogHeight: 188
    readonly property int dialogRadius: 10
    readonly property int dialogTopMargin: 24
    readonly property int dialogHorizontalMargin: 16
    readonly property int dialogContentHeight: 96
    readonly property int dialogContentSpacing: 8

    property alias titleText: title.text
    property alias bodyText: body.text
    property alias cancelButtonText: cancelButton.text
    property alias acceptButtonText: acceptButton.text
    property int modelCount: 0

    signal accepted

    width: dialogWidth
    height: dialogHeight
    flags: Qt.Dialog | Qt.FramelessWindowHint
    modality: Qt.WindowModal
    color: "transparent"

    Rectangle {
        id: background

        anchors.fill: parent
        color: ColorTheme.surface1
        radius: dialogRadius

        Column {
            id: mainColumn

            anchors {
                fill: parent
                topMargin: dialogTopMargin
                leftMargin: dialogHorizontalMargin
                rightMargin: dialogHorizontalMargin
            }

            RowLayout {
                id: rowLayout

                width: mainColumn.width
                height: dialogContentHeight
                spacing: dialogHorizontalMargin

                Image {
                    id: leftImage

                    source: Images.syncsRemoveConfirm
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

                        Layout.preferredWidth: 416
                        lineHeightMode: Text.FixedHeight
                        lineHeight: 18
                        font.pixelSize: Texts.Text.Size.NORMAL
                    }
                }

            }

            Item {
                id: buttonsLayout

                anchors {
                    right: mainColumn.right
                    rightMargin: Constants.focusAdjustment
                    left: mainColumn.left
                }
                height: acceptButton.height + 2 * acceptButton.sizes.focusBorderWidth

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
                        root.accepted();
                    }
                }
            }
        }
    }

}
