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
    readonly property int titleLineHeight: 30
    readonly property int descriptionLineHeight: 24
    readonly property int minButtonsSize: 80
    readonly property int mainColSpacing: 13

    property alias titleText: title.text
    property alias bodyText: body.text
    property alias cancelButtonText: cancelButton.text
    property alias acceptButtonText: acceptButton.text
    property int modelCount: 0

    signal accepted

    width: dialogWidth
    minimumWidth: dialogWidth
    maximumWidth: dialogWidth
    height: dialogHeight
    minimumHeight: dialogHeight
    maximumHeight: dialogHeight
    flags: Qt.Dialog | (isWindows ? Qt.CustomizeWindowHint : Qt.FramelessWindowHint)
    modality: Qt.WindowModal
    color: isWindows ? ColorTheme.surface1 : "transparent"

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
                bottomMargin: dialogTopMargin
                leftMargin: dialogHorizontalMargin
                rightMargin: dialogHorizontalMargin
            }

            spacing: mainColSpacing

            RowLayout {
                id: rowLayout

                width: parent.width
                height: dialogContentHeight
                spacing: dialogHorizontalMargin

                Image {
                    id: leftImage

                    source: Images.syncsRemoveConfirm
                    sourceSize: Qt.size(dialogContentHeight, dialogContentHeight)
                }

                ColumnLayout {
                    id: contentLayout

                    Layout.fillWidth: true
                    spacing: dialogContentSpacing

                    Texts.Text {
                        id: title

                        Layout.fillWidth: true
                        lineHeightMode: Text.FixedHeight
                        lineHeight: titleLineHeight
                        font {
                            pixelSize: Texts.Text.Size.LARGE
                            weight: Font.DemiBold
                        }
                    }

                    Texts.Text {
                        id: body

                        Layout.fillWidth: true
                        lineHeightMode: Text.FixedHeight
                        lineHeight: descriptionLineHeight
                        font.pixelSize: Texts.Text.Size.MEDIUM_LARGE
                    }
                }
            }

            Row {
                id: buttonRow

                anchors.right: mainColumn.right
                anchors.rightMargin: Constants.focusAdjustment
                layoutDirection: Qt.RightToLeft

                PrimaryButton {
                    id: acceptButton

                    icons.source: Images.cross
                    icons.position: Icon.Position.LEFT

                    onClicked: {
                        root.accepted();
                    }
                }

                OutlineButton {
                    id: cancelButton

                    visible: true

                    onClicked: {
                        root.close();
                    }
                }
            }
        }
    }

}
