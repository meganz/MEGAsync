import QtQuick 2.15

import syncs 1.0
import SyncsComponents 1.0
import SyncInfo 1.0

import ChooseLocalFolder 1.0
import ChooseRemoteFolder 1.0

AddSyncPageForm {
    id: root

    signal moveBack
    signal moveNext

    onVisibleChanged: {
        syncsComponentAccess.clearLocalFolderHint();
        syncsComponentAccess.clearRemoteFolderHint();
    }

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
        syncsComponentAccess.clearLocalFolderHint();
        localFolderSelector.openFolderSelector(localFolderChooser.chosenPath);
    }

    remoteFolderChooser.onButtonClicked: {
        syncsComponentAccess.clearRemoteFolderHint();
        remoteFolderSelector.openFolderSelector();
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
            syncsComponentAccess.addSyncCandidadeButtonClicked(localFolderChooser.chosenPath, remoteFolderChooser.chosenPath);
        }
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
            remoteFolderChooser.chosenPath = remoteFolderPath;
        }
    }

    Connections {
        id: localFolderChooserConnection

        target: localFolderSelector
        enabled: root.enabled

        function onFolderChosen(localFolderPath) {
            localFolderChooser.chosenPath = localFolderPath;
        }
    }

    Connections {
        target: syncsDataAccess

        function onSyncPrevalidationSuccess() {
            enableScreen();
            root.moveNext();
        }

        function onSyncPrevalidationFailed() {
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
