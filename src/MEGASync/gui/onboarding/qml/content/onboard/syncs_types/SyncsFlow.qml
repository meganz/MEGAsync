import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Onboard.Syncs_types.Left_panel 1.0
import Onboard.Syncs_types.Syncs 1.0
import Onboard.Syncs_types.Backups 1.0

import Components 1.0 as Custom
import Common 1.0

StackView {
    id: syncsFlow

    readonly property string computerName: "computerName"
    readonly property string syncType: "syncType"
    readonly property string syncs: "syncs"
    readonly property string selectiveSync: "selectiveSync"
    readonly property string fullSync: "fullSync"
    readonly property string selectBackup: "selectBackup"
    readonly property string confirmBackup: "confirmBackup"
    readonly property string finalState: "finalState"

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
                script: {
                    rightPanel.replace(syncPage);
                    if(syncsFlow.currentItem != syncsPanel) {
                        syncsFlow.replace(syncsPanel);
                    }
                }
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
                script: {
                    rightPanel.replace(selectBackupFoldersPage);
                    if(syncsFlow.currentItem != syncsPanel) {
                        syncsFlow.replace(syncsPanel);
                    }
                }
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
                script: syncsFlow.replace(finalPage);
            }
        }
    ]

    ResumePage {
        id: finalPage

        buttonGroup.onClicked: {
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
        }
        visible: false
    }

    Rectangle {
        id: syncsPanel

        width: syncsFlow.width
        height: syncsFlow.height

        StepPanel {
            id: stepPanel

            z: 2
            width: 224
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
        }

        StackView {
            id: rightPanel

            anchors {
                left: stepPanel.right
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }

            ComputerNamePage {
                id: computerNamePage

                Layout.fillHeight: true
                Layout.fillWidth: true
                footerButtons {
                    previousButton.visible: false
                    notNowButton.visible: false
                    nextButton.onClicked: {
                        syncsFlow.state = syncType;
                    }
                }
                visible: false
            }

            InstallationTypePage {
                id: installationTypePage

                footerButtons {
                    previousButton.onClicked: {
                        syncsFlow.state = computerName;
                    }

                    nextButton.onClicked: {
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
                }
                visible: false
            }

            SyncTypePage {
                id: syncPage

                footerButtons {
                    previousButton.onClicked: {
                        syncsFlow.state = syncType;
                    }

                    nextButton.onClicked: {
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
                }
                visible: false
            }

            FullSyncPage {
                id: fullSyncPage

                footerButtons{
                    previousButton.onClicked: {
                        syncsFlow.state = syncs;
                    }
                    nextButton.onClicked: {
                        syncsFlow.state = finalState;
                    }
                }
                visible: false
            }

            SelectiveSyncPage {
                id: selectiveSyncPage

                footerButtons{
                    previousButton.onClicked: {
                        syncsFlow.state = syncs;
                    }
                    nextButton.onClicked: {
                        syncsFlow.state = finalState;
                    }
                }
                visible: false
            }

            SelectFoldersPage {
                id: selectBackupFoldersPage

                footerButtons {
                    previousButton.onClicked: {
                        syncsFlow.state = syncType;
                    }

                    nextButton.enabled: false
                    nextButton.onClicked: {
                        syncsFlow.state = confirmBackup;
                    }
                }
                visible: false
            }

            ConfirmFoldersPage {
                id: confirmBackupFoldersPage

                footerButtons {
                    previousButton.onClicked: {
                        syncsFlow.state = selectBackup;
                    }

                    nextButton.onClicked: {
                        syncsFlow.state = finalState;
                    }
                }
                visible: false
            }
        }
    }
}
