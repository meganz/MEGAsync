import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.checkBoxes 1.0
import components.pages 1.0

import syncs 1.0
import SyncsComponents 1.0
import SyncInfo 1.0

Window {
    id: root

    property alias enabled : addSyncForm.enabled

    width: addSyncForm.width
    height: addSyncForm.height
    flags: Qt.Dialog | Qt.FramelessWindowHint
    modality: Qt.WindowModal
    color: "transparent"

    AddSyncDialogForm {
        id: addSyncForm

        localFolderChooser.folderField {
            hint {
                text: syncsDataAccess.localError
                visible: syncsDataAccess.localError.length !== 0
            }
            error: syncsDataAccess.localError.length !== 0
            text: syncsDataAccess.localFolderCandidate
        }

        remoteFolderChooser.folderField {
            hint {
                text: syncsDataAccess.remoteError
                visible: syncsDataAccess.remoteError.length !== 0
            }
            error: syncsDataAccess.remoteError.length !== 0
            text: syncsDataAccess.remoteFolderCandidate
        }

        localFolderChooser.onButtonClicked: {
            syncsComponentAccess.chooseLocalFolderButtonClicked();
        }

        remoteFolderChooser.onButtonClicked: {
            syncsComponentAccess.chooseRemoteFolderButtonClicked();
        }

        rightPrimaryButton.onClicked: {
            root.enabled = false;
            rightPrimaryButton.icons.busyIndicatorVisible = true;
            syncsComponentAccess.preSyncValidationButtonClicked();
        }

        rightSecondaryButton.onClicked: {
            addSyncForm.enableScreen();
            syncsComponentAccess.closeDialogButtonClicked();
            root.close();
        }

        function enableScreen() {
            root.enabled = true;
            rightPrimaryButton.icons.busyIndicatorVisible = false;
        }

        Connections {
            target: syncsDataAccess

            function onSyncPrevalidationSuccess() {
                addSyncForm.enableScreen();
                root.close();
            }

            function onSyncPrevalidationFailed() {
                addSyncForm.enableScreen();
            }
        }
    }
}
