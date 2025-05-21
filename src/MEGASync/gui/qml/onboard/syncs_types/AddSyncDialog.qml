import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.checkBoxes 1.0
import components.pages 1.0

import syncs 1.0

Window {
    id: root

    readonly property int dialogMargin: 24
    readonly property int dialogWidth: 453
    readonly property int dialogHeight: 344
    readonly property int buttonsSpacing: 12
    readonly property int dialogRadius: 10
    readonly property int defaultSpacing: 32
    readonly property int defaultTopMargin: 16

    width: root.dialogWidth
    height: root.dialogHeight
    flags: Qt.Dialog | Qt.FramelessWindowHint
    modality: Qt.WindowModal
    color: "transparent"

    Rectangle {
        id: mainArea

        color: ColorTheme.surface1
        radius: dialogRadius

        anchors {
            fill: parent
        }

        Texts.RichText {
            id: title

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: dialogMargin
            }

            lineHeightMode: Text.FixedHeight
            lineHeight: 24
            font {
                pixelSize: Texts.Text.Size.MEDIUM_LARGE
                weight: Font.DemiBold
            }
            text: qsTr("Select folders to sync")
        }

        ColumnLayout {
            id: column

            anchors {
                top: title.bottom
                left: parent.left
                right: parent.right
                margins: dialogMargin
            }
            spacing: Constants.defaultComponentSpacing

            ChooseSyncFolder {
                id: localFolder

                title: SyncsStrings.selectLocalFolder
                leftIconSource: Images.pc
                chosenPath: syncsDataAccess.defaultLocalFolder
                Layout.preferredWidth: parent.width
                Layout.topMargin: root.defaultTopMargin
            }

            ChooseSyncFolder {
                id: remoteFolder

                title: SyncsStrings.selectMEGAFolder
                leftIconSource: Images.megaOutline
                chosenPath: syncsDataAccess.defaultRemoteFolder
                Layout.preferredWidth: parent.width
            }

            Row {
                id: buttonRow

                Layout.topMargin: root.defaultTopMargin
                Layout.alignment: Qt.AlignRight
                layoutDirection: Qt.RightToLeft
                spacing: root.buttonsSpacing

                PrimaryButton {
                    id: acceptButton

                    text: qsTr("Add")
                    onClicked: {
                        accepted();
                        root.close();
                    }
                }

                OutlineButton {
                    id: cancelButton

                    text: qsTr("Cancel")
                    visible: true
                    onClicked: {
                        rejected();
                        root.close();
                    }
                }
            }
        }

    }
}
