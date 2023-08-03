// System
import QtQuick 2.12

// C++
import BackupsController 1.0
import BackupsModel 1.0

ConfirmFoldersPageForm {
    id: root

    property bool existsSDKError: false

    footerButtons {

        rightSecondary.onClicked: {
            backupsProxyModel.selectedFilterEnabled = false;
            backupsFlow.state = backupsFlow.selectBackup;
        }

        rightPrimary.onClicked: {
            root.existsSDKError = false;
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            backupsProxyModel.createBackups();
        }
    }

    Connections {
        target: BackupsModel

        onNoneSelected: {
            footerButtons.rightSecondary.clicked();
        }

        onExistConflictsChanged: {
            if(!root.existsSDKError) {
                if(BackupsModel.mConflictsNotificationText !== "") {
                    stepPanel.state = stepPanel.step4Warning;
                } else {
                    stepPanel.state = stepPanel.step4;
                }
            }
        }
    }

    Connections {
        target: BackupsController

        onBackupsCreationFinished: (success, message) => {
            root.existsSDKError = !success;
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            if(success) {
                syncsPanel.state = syncsPanel.finalState;
            } else {
                stepPanel.state = stepPanel.step4Error;
            }
        }
    }

}
