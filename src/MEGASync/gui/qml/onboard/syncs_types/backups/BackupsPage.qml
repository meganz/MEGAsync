import QtQuick 2.15

import backups 1.0

import onboard 1.0
import onboard.syncs_types 1.0

import BackupsComponent 1.0
import BackupCandidates 1.0

BackupsFlow {
    id: root

    required property StepPanel stepPanelRef

    BackupsComponent
    {
        id: backupsComponentAccess
    }

    selectFoldersPage: Component {
        id: selectFoldersPageComponent

        SelectFoldersPage {
            id: selectFoldersPageItem
        }
    }

    confirmFoldersPage: Component {
        id: confirmFoldersPageComponent

        ConfirmFoldersPage {
            id: confirmFoldersPageItem
            backupsComponentAccessRef: backupsComponentAccess
        }
    }

    Item {
        id: stepPanelStateWrapper

        readonly property string selectBackupPage: "selectBackupPage"
        readonly property string confirmBackupPage: "confirmBackupPage"

        states: [
            State {
                name: stepPanelStateWrapper.selectBackupPage
                PropertyChanges {
                    target: root.stepPanelRef;
                    state: root.stepPanelRef.step3;
                    step3Text: OnboardingStrings.backupSelectFolders;
                    step4Text: OnboardingStrings.confirm;
                }
            },
            State {
                name: stepPanelStateWrapper.confirmBackupPage
                PropertyChanges {
                    target: root.stepPanelRef;
                    state: {
                        if(backupsComponentAccess.data.globalError !== BackupCandidates.NONE) {
                            if(backupsComponentAccess.data.globalError === BackupCandidates.SDK_CREATION) {
                                return root.stepPanelRef.step4Error;
                            }
                            else {
                                return root.stepPanelRef.step4Warning;
                            }
                        }
                        else {
                            return root.stepPanelRef.step4;
                        }
                    }
                    step3Text: OnboardingStrings.backupSelectFolders;
                    step4Text: OnboardingStrings.backupConfirm;
                }
            }
        ]
    }

    onStateChanged: {
        switch(root.state) {
            case root.selectBackup:
                stepPanelStateWrapper.state = stepPanelStateWrapper.selectBackupPage;
                break;
            case root.confirmBackup:
                stepPanelStateWrapper.state = stepPanelStateWrapper.confirmBackupPage;
                break;
            default:
                console.warn("BackupsPage: state does not exist -> " + root.state);
                break;
        }
    }

}
