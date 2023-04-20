// System
import QtQuick 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

ConfirmFoldersPageForm {

    property bool success: false

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = selectBackup;
        }

        nextButton {
            text: OnboardingStrings.backup
            iconSource: Images.cloud
            onClicked: {
                success = false;
                backupTable.backupModel.updateConfirmed();
                Onboarding.addBackups(backupTable.backupModel.getConfirmedDirs());
            }
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

            success = success & (errorCode === 0);
            //if(finished && success) {
                syncsFlow.state = finalState;
            //} else {
            //    syncsFlow.state = renameBackupFolder;
            //}
        }
    }

}
