import QtQuick 2.15

import syncs 1.0
import SyncsComponents 1.0
import SyncInfo 1.0

AddSyncPageForm {
    id: root

    signal moveBack
    signal moveNext

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

    function enableScreen() {
        root.enabled = true;
        footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
    }

    footerButtons {
        leftSecondary.onClicked: {
            syncsComponentAccess.exclusionsButtonClicked();
        }

        rightSecondary.onClicked: {
            root.moveBack();
        }

        rightPrimary.onClicked: {
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            syncsComponentAccess.preSyncValidationButtonClicked();
        }
    }

    Connections {
        target: syncsDataAccess

        function onSyncPrevalidationSuccess() {
            enableScreen();

            root.moveNext();
        }

        function onLocalErrorChanged() {
            enableScreen();
        }

        function onRemoteErrorChanged() {
            enableScreen();
        }
    }

    Connections {
        target: window

        function onInitializePageFocus() {
            localFolderChooser.forceActiveFocus();
        }
    }
}
