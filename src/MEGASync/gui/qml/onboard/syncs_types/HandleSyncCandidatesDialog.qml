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

import ChooseLocalFolder 1.0
import ChooseRemoteFolder 1.0

Window {
    id: root

    property alias enabled: handleSyncCandidateForm.enabled
    property alias rightPrimaryButton: handleSyncCandidateForm.rightPrimaryButton
    property alias title: handleSyncCandidateForm.dialogTitle
    property string editLocalPath
    property string editRemotePath

    readonly property int dialogWidth: 453
    readonly property int dialogHeight: 344

    width: dialogWidth
    minimumWidth: dialogWidth
    maximumWidth: dialogWidth

    minimumHeight: dialogHeight
    height: dialogHeight
    maximumHeight: dialogHeight

    flags: Qt.Dialog | (isWindows ? Qt.CustomizeWindowHint : Qt.FramelessWindowHint)
    modality: Qt.WindowModal
    color: isWindows ? ColorTheme.surface1 : "transparent"

    onVisibleChanged: {
        if (editLocalPath !== "") {
            handleSyncCandidateForm.localFolderChooser.chosenPath = editLocalPath
        }
        else {
            handleSyncCandidateForm.localFolderChooser.chosenPath = ""
        }

        if (editRemotePath !== "") {
            handleSyncCandidateForm.remoteFolderChooser.chosenPath = editRemotePath
        }
        else {
            handleSyncCandidateForm.remoteFolderChooser.chosenPath = ""
        }
    }

    HandleSyncCandidatesDialogForm {
        id: handleSyncCandidateForm

        localFolderChooser.folderField {
            hint {
                text: syncsDataAccess.localError
                visible: syncsDataAccess.localError.length !== 0
            }
            error: syncsDataAccess.localError.length !== 0
        }

        remoteFolderChooser.folderField {
            hint {
                text: syncsDataAccess.remoteError
                visible: syncsDataAccess.remoteError.length !== 0
            }
            error: syncsDataAccess.remoteError.length !== 0
        }

        localFolderChooser.onButtonClicked: {
            localFolderSelector.openFolderSelector(localFolderChooser.chosenPath);
        }

        remoteFolderChooser.onButtonClicked: {
            remoteFolderSelector.openFolderSelector();
        }

        rightPrimaryButton.onClicked: {
            root.enabled = false;

            if (editLocalPath !== "" && editRemotePath !== "")
            {
                syncsComponentAccess.editSyncCandidadeButtonClicked(handleSyncCandidateForm.localFolderChooser.chosenPath, handleSyncCandidateForm.remoteFolderChooser.chosenPath, editLocalPath, editRemotePath);
            }
            else
            {
                syncsComponentAccess.addSyncCandidadeButtonClicked(handleSyncCandidateForm.localFolderChooser.chosenPath, handleSyncCandidateForm.remoteFolderChooser.chosenPath);
            }
        }

        rightSecondaryButton.onClicked: {
            handleSyncCandidateForm.enableScreen();
            syncsComponentAccess.closeDialogButtonClicked();
            root.close();
        }

        ChooseLocalFolder {
            id: localFolderSelector
        }

        ChooseRemoteFolder {
            id: remoteFolderSelector
        }

        Connections {
            id: remoteFolderChooserConnection

            target: remoteFolderSelector
            enabled: root.enabled

            function onFolderChosen(remoteFolderPath) {
                syncsComponentAccess.clearRemoteFolderErrorHint();
                handleSyncCandidateForm.remoteFolderChooser.chosenPath = remoteFolderPath;
            }
        }

        Connections {
            id: localFolderChooserConnection

            target: localFolderSelector
            enabled: root.enabled

            function onFolderChosen(localFolderPath) {
                syncsComponentAccess.clearLocalFolderErrorHint();
                handleSyncCandidateForm.localFolderChooser.chosenPath = localFolderPath;
            }
        }

        function enableScreen() {
            root.enabled = true;
        }

        Connections {
            target: syncsDataAccess

            function onSyncPrevalidationSuccess() {
                handleSyncCandidateForm.enableScreen();
                root.close();
            }

            function onSyncPrevalidationFailed() {
                handleSyncCandidateForm.enableScreen();
            }
        }
    }
}
