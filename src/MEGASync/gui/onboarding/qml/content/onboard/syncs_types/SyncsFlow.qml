import QtQuick 2.12
import QtQuick.Controls 2.12

SyncsFlowForm {
    id: syncsFlow

    state: computerName

    states: [
        State {
            name: computerName
            StateChangeScript {
                script: rightPanel.replace(computerNamePage);
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.step1ComputerName
            }
        },
        State {
            name: syncType
            StateChangeScript {
                script: rightPanel.replace(installationTypePage);
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.step2InstallationType
            }
        },
        State {
            name: syncs
            StateChangeScript {
                script: rightPanel.replace(syncPage);
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepSelectSyncType
            }
        },
        State {
            name: selectiveSync
            StateChangeScript {
                script: rightPanel.replace(selectiveSyncPage);
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepSyncFolder
            }
        },
        State {
            name: fullSync
            StateChangeScript {
                script: rightPanel.replace(fullSyncPage);
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepSyncFolder
            }
        },
        State {
            name: selectBackup
            StateChangeScript {
                script: rightPanel.replace(selectBackupFoldersPage);
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepBackupsSelectFolders
            }
        },
        State {
            name: confirmBackup
            StateChangeScript {
                script: rightPanel.replace(confirmBackupFoldersPage);
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepBackupsConfirm
            }
        },
        State {
            name: finalState
            StateChangeScript {
                script: mainPanel.replace(finalPage);
            }
        }
    ]

    computerNamePage.footerButtons.previousButton.visible: false

    computerNamePage.footerButtons.notNowButton.visible: false

    computerNamePage.footerButtons.nextButton.onClicked: {
        syncsFlow.state = syncType;
    }

    installationTypePage.footerButtons.previousButton.onClicked: {
        syncsFlow.state = computerName;
    }

    installationTypePage.footerButtons.nextButton.onClicked: {
        switch(installationTypePage.buttonGroup.checkedButton.type) {
            case InstallationTypeButton.Type.Sync:
                syncsFlow.state = syncs;
                break;
            case InstallationTypeButton.Type.Backup:
                syncsFlow.state = selectBackup;
                break;
            case InstallationTypeButton.Type.Fuse:
            default:
                console.error("Button type does not exist -> "
                              + installationTypePage.buttonGroup.checkedButton.type);
                break;
        }
    }

    syncPage.footerButtons.previousButton.onClicked: {
        syncsFlow.state = syncType;
    }

    syncPage.footerButtons.nextButton.onClicked: {
        switch(syncPage.buttonGroup.checkedButton.type) {
            case ResumeButton.Type.FullSync:
                syncsFlow.state = fullSync;
                break;
            case ResumeButton.Type.SelectiveSync:
                syncsFlow.state = selectiveSync;
                break;
            default:
                console.error("Button type does not exist -> "
                              + syncPage.buttonGroup.checkedButton.type);
                break;
        }
    }

    selectiveSyncPage.footerButtons.previousButton.onClicked: {
        syncsFlow.state = syncs;
    }

    fullSyncPage.footerButtons.previousButton.onClicked: {
        syncsFlow.state = syncs;
    }

    selectiveSyncPage.footerButtons.nextButton.onClicked: {
        syncsFlow.state = finalState;
    }

    fullSyncPage.footerButtons.nextButton.onClicked: {
        syncsFlow.state = finalState;
    }

    selectBackupFoldersPage.footerButtons.previousButton.onClicked: {
        syncsFlow.state = syncType;
    }

    selectBackupFoldersPage.footerButtons.nextButton.enabled: false

    selectBackupFoldersPage.footerButtons.nextButton.onClicked: {
        syncsFlow.state = confirmBackup;
    }

    confirmBackupFoldersPage.footerButtons.previousButton.onClicked: {
        syncsFlow.state = selectBackup;
    }

    confirmBackupFoldersPage.footerButtons.nextButton.onClicked: {
        syncsFlow.state = finalState;
    }

    finalPage.buttonGroup.onClicked: {
        switch(button.type) {
            case InstallationTypeButton.Type.Sync:
                syncsFlow.state = syncs;
                break;
            case InstallationTypeButton.Type.Backup:
                syncsFlow.state = selectBackup;
                break;
            case InstallationTypeButton.Type.Fuse:
                break;
            default:
                console.error("Button type does not exist -> " + button.type);
                break;
        }
        mainPanel.replace(syncsPanel);
    }
}
