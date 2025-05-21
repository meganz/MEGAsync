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

    property alias enabled : mainArea.enabled

    width: root.dialogWidth
    height: mainArea.height
    flags: Qt.Dialog | Qt.FramelessWindowHint
    modality: Qt.WindowModal
    color: "transparent"

    function enableScreen() {
        root.enabled = true;
        acceptButton.icons.busyIndicatorVisible = false;
    }

    Rectangle {
        id: mainArea

        color: ColorTheme.surface1
        radius: dialogRadius
        width: parent.width
        height: column.implicitHeight + 2 * dialogMargin

        ColumnLayout {
            id: column

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: dialogMargin
            }

            spacing: Constants.defaultComponentSpacing

            Texts.RichText {
                id: title

                lineHeightMode: Text.FixedHeight
                lineHeight: 24
                font {
                    pixelSize: Texts.Text.Size.MEDIUM_LARGE
                    weight: Font.DemiBold
                }
                text: qsTr("Select folders to sync")
            }

            ChooseSyncFolder {
                id: localFolder

                title: SyncsStrings.selectLocalFolder
                leftIconSource: Images.pc
                chosenPath: syncsDataAccess.defaultLocalFolder
                Layout.preferredWidth: parent.width
                Layout.topMargin: root.defaultTopMargin

                onButtonClicked: {
                    syncsComponentAccess.chooseLocalFolderButtonClicked();
                }

                folderField {
                    hint {
                        text: syncsDataAccess.localError
                        visible: syncsDataAccess.localError.length !== 0
                    }
                    error: syncsDataAccess.localError.length !== 0
                    text: syncsDataAccess.localFolderCandidate
                }
            }

            ChooseSyncFolder {
                id: remoteFolder

                title: SyncsStrings.selectMEGAFolder
                leftIconSource: Images.megaOutline
                chosenPath: syncsDataAccess.defaultRemoteFolder
                Layout.preferredWidth: parent.width

                onButtonClicked: {
                    syncsComponentAccess.chooseRemoteFolderButtonClicked();
                }

                folderField {
                    hint {
                        text: syncsDataAccess.remoteError
                        visible: syncsDataAccess.remoteError.length !== 0
                    }
                    error: syncsDataAccess.remoteError.length !== 0
                    text: syncsDataAccess.remoteFolderCandidate
                }
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
                        root.enabled = false;
                        acceptButton.icons.busyIndicatorVisible = true;
                        syncsComponentAccess.preSyncValidationButtonClicked();
                    }
                }

                OutlineButton {
                    id: cancelButton

                    text: qsTr("Cancel")
                    visible: true
                    onClicked: {
                        root.close();
                    }
                }
            }
        }
    }

    Connections {
        target: syncsDataAccess

        function onSyncPrevalidationSuccess() {
            enableScreen();
            root.close();
        }

        function onLocalErrorChanged() {
            enableScreen();
        }

        function onRemoteErrorChanged() {
            enableScreen();
        }
    }
}
