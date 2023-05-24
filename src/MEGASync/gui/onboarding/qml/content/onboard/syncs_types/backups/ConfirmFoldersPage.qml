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
            backupsFlow.state = backupsFlow.selectBackup;
        }

        nextButton.onClicked: {
            success = false;
            root.enabled = false;
            footerButtons.nextButton.icons.busyIndicatorVisible = true;
            proxyModel.updateConfirmed();
            Onboarding.addBackups(proxyModel.getConfirmedDirs());
        }
    }

    Component.onCompleted: {
        proxyModel.selectedFilterEnabled = true;
    }

    Connections {
        target: Onboarding

        onBackupsUpdated: (path, errorCode, finished) => {
            proxyModel.update(path, errorCode);

            if(finished) {
                proxyModel.clean();
                root.enabled = true;
                footerButtons.nextButton.icons.busyIndicatorVisible = false;
                syncsFlow.state = syncsFlow.finalState;
            }
        }

        onBackupConflict: (folder, name, isNew) => {
            proxyModel.clean();
            root.enabled = true;
            footerButtons.nextButton.icons.busyIndicatorVisible = false;
        }
    }

}
