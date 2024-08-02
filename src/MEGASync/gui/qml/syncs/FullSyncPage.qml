import QtQuick 2.15

import Syncs 1.0
import ChooseLocalFolder 1.0

FullSyncPageForm {
    id: root

    signal fullSyncMoveToBack
    signal fullSyncMoveToSuccess

    localFolderChooser.folderField.hint.text : root.syncs.localError
    localFolderChooser.folderField.hint.visible : root.syncs.localError.length !== 0
    localFolderChooser.folderField.error : root.syncs.localError.length !== 0

    function enableScreen() {
        root.enabled = true;
        footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
    }

    footerButtons {
        leftSecondary.onClicked: {
            if(!root.isOnboarding) {
                syncsComponentAccess.openExclusionsDialog(localFolderChooser.choosenPath);
            }
        }

        rightSecondary.onClicked: {
            root.fullSyncMoveToBack();
        }

        rightPrimary.onClicked: {
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;

            root.syncs.addSync(localFolderChooser.choosenPath);
        }
    }

    ChooseLocalFolder {
        id: localFolder
    }

    Connections {
        target: root.syncs

        function onSyncSetupSuccess() {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            root.fullSyncMoveToSuccess();
            localFolderChooser.reset();
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
