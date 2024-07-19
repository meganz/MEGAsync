import QtQuick 2.15

import Syncs 1.0
import ChooseLocalFolder 1.0

FullSyncPageForm {
    id: root

    signal fullSyncMoveToBack
    signal fullSyncMoveToSuccess

    localFolderChooser.folderField.hint.text : syncs.localError
    localFolderChooser.folderField.hint.visible : syncs.localError.length !== 0
    localFolderChooser.folderField.error : syncs.localError.length !== 0

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

            syncs.addSync(localFolderChooser.choosenPath);
        }
    }

    ChooseLocalFolder {
        id: localFolder
    }

    Syncs {
        id: syncs

        onSyncSetupSuccess: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            root.fullSyncMoveToSuccess();
            localFolderChooser.reset();
        }

        onLocalErrorChanged: {
            enableScreen();
        }

        onRemoteErrorChanged: {
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
