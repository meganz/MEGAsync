import QtQuick 2.15

import common 1.0

import backups 1.0 as Backup
import syncs 1.0

import onboard 1.0

ResumePageForm {
    id: root

    property int tabToOpen: 0
    property bool fullSyncDone: false
    property bool selectiveSyncDone: false

    signal resumePageMoveToSyncs
    signal resumePageMoveToSelectiveSyncs
    signal resumePageMoveToBackup

    readonly property string stateFullSync: "stateFullSync"
    readonly property string stateSelectiveSync: "stateSelectiveSync"
    readonly property string stateBackup: "stateBackup"
    readonly property string stateInitial: "stateInitial"

    required property StepPanel stepPanelRef
    required property NavigationInfo navInfoRef

    buttonGroup.onClicked: {
        switch(button.type) {
            case Constants.SyncType.SYNC:
                root.resumePageMoveToSyncs();
                break;
            case Constants.SyncType.SELECTIVE_SYNC:
                root.resumePageMoveToSelectiveSyncs();
                break;
            case Constants.SyncType.BACKUP:
                root.resumePageMoveToBackup();
                break;
            case Constants.SyncType.MIGRATE:
                onboardingAccess.openMigrationTool();
                break;
            default:
                console.error("Button type does not exist -> " + button.type);
                break;
        }
    }

    footerButtons {
        rightSecondary.onClicked: {
            onboardingAccess.openPreferences(tabToOpen);
        }

        rightPrimary.onClicked: {
            window.close();
        }
    }

    states: [
        State {
            name: root.stateFullSync

            PropertyChanges { target: titleItem; restoreEntryValues: true; text: SyncsStrings.finalStepSyncTitle; }
            PropertyChanges { target: descriptionItem; visible: true; }
            PropertyChanges { target: descriptionItem2; visible: false; }
            PropertyChanges {
                target: syncButton;
                type: Constants.SyncType.BACKUP;
                title: OnboardingStrings.backup;
                description: OnboardingStrings.finalPageButtonBackup;
                imageSource: Images.installationTypeBackups;
            }
            PropertyChanges {
                target: backupButton;
                type: Constants.SyncType.MIGRATE;
                title: OnboardingStrings.importFromAnotherCloud;
                description: OnboardingStrings.finalPageButtonImportFromCloud;
                imageSource: Images.importFromCloud;
            }
            PropertyChanges {
                target: stepPanelRef;
                state: stepPanelRef.stepAllDone;
                step3Text: SyncsStrings.selectFolders;
                step4Text: OnboardingStrings.syncSetUp;
            }
        },

        State {
            name: root.stateSelectiveSync
            extend: root.stateFullSync
        },

        State {
            name: root.stateBackup

            PropertyChanges { target: titleItem; text: Backup.BackupsStrings.finalStepBackupTitle; }
            PropertyChanges { target: descriptionItem; text: Backup.BackupsStrings.finalStepBackup; }
            PropertyChanges {
                target: descriptionItem2;
                text: Backup.BackupsStrings.finalStepBackup2;
                visible: true;
            }
            PropertyChanges {
                target: syncButton;
                type: !fullSyncDone && !selectiveSyncDone
                      ? Constants.SyncType.SYNC
                      : Constants.SyncType.SELECTIVE_SYNC;
                visible: !fullSyncDone;
                title: SyncsStrings.sync
                description: !fullSyncDone && !selectiveSyncDone
                             ? OnboardingStrings.finalPageButtonSync
                             : OnboardingStrings.finalPageButtonSelectiveSync;
            }
            PropertyChanges {
                target: backupButton;
                type: Constants.SyncType.MIGRATE;
                title: OnboardingStrings.importFromAnotherCloud;
                description: OnboardingStrings.finalPageButtonImportFromCloud;
                imageSource: Images.importFromCloud;
            }
            PropertyChanges {
                target: stepPanelRef;
                state: stepPanelRef.stepAllDone;
                step3Text: OnboardingStrings.backupSelectFolders;
                step4Text: OnboardingStrings.backupConfirm;
            }
        }
    ]

    Connections {
        target: window

        function onInitializePageFocus() {
            syncButton.forceActiveFocus();
        }
    }
}
