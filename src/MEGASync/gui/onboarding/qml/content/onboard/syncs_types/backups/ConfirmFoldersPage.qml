// System
import QtQuick 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

ConfirmFoldersPageForm {
    id: root

    property bool success: false

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = syncsFlow.selectBackup;
        }

        nextButton.onClicked: {
            success = false;
            root.enabled = false;
            footerButtons.nextButton.icons.busyIndicatorVisible = true;
            backupTable.backupModel.updateConfirmed();
            Onboarding.addBackups(backupTable.backupModel.getConfirmedDirs());
        }
    }

    onVisibleChanged: {
        if(visible) {
            backupTable.backupProxyModel.selectedFilterEnabled = true;
        }
    }

    Connections {
        target: Onboarding

        onBackupsUpdated: (path, errorCode, finished) => {
            backupTable.backupModel.update(path, errorCode);

            if(finished) {
                backupTable.backupModel.clean();
                root.enabled = true;
                footerButtons.nextButton.busyIndicatorVisible = false;
                syncsFlow.state = syncsFlow.finalState;
            }
        }

        onBackupConflict: (folder, name, isNew) => {
            backupTable.backupModel.clean();
            root.enabled = true;
            footerButtons.nextButton.icons.busyIndicatorVisible = false;
        }
    }

}
