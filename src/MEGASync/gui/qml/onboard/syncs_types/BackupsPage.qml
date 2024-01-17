import QtQuick 2.15

import backups 1.0

import onboard 1.0
import onboard.syncs_types.left_panel 1.0

import BackupsModel 1.0

BackupsFlow {
    id: root

    required property StepPanel stepPanelRef

    isOnboarding: true

    onStateChanged: {
        switch(root.state) {
            case root.selectBackup:
                stepPanelRef.step3Text = OnboardingStrings.backupSelectFolders;
                stepPanelRef.step4Text = OnboardingStrings.confirm;
                stepPanelRef.state = stepPanelRef.step3;
                break;
            case root.confirmBackup:
                if(backupsModelAccess.conflictsNotificationText !== "") {
                    stepPanelRef.state = stepPanelRef.step4Warning;
                }
                else {
                    stepPanelRef.state = stepPanelRef.step4;
                }
                stepPanelRef.step3Text = OnboardingStrings.backupSelectFolders;
                stepPanelRef.step4Text = OnboardingStrings.backupConfirm;
                stepPanelRef.state = stepPanelRef.step4;
                break;
            default:
                console.warn("BackupsPage: state does not exist -> " + root.state);
                break;
        }
    }

    onBackupFlowMoveToFinal: (success) => {
        if(!success) {
            stepPanelRef.state = stepPanelRef.step4Error;
        }
    }

    Connections {
        id: backupsModelConnections

        target: backupsModelAccess
        ignoreUnknownSignals: true

        function onGlobalErrorChanged() {
            if(backupsModelAccess.globalError !== backupsModelAccess.BackupErrorCode.NONE) {
                if(backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.SDK_CREATION) {
                    stepPanelRef.state = stepPanelRef.step4Error;
                }
                else {
                    stepPanelRef.state = stepPanelRef.step4Warning;
                }
            }
            else if (root.state == root.confirmBackup) {
                stepPanelRef.state = stepPanelRef.step4;
            }
        }
    }
}
