// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components 1.0 as Custom
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types.Left_panel 1.0
import Onboard.Syncs_types.Syncs 1.0
import Onboard.Syncs_types.Backups 1.0

// C++
import BackupsProxyModel 1.0

StackView {
    id: syncsFlow

    readonly property string computerName: "computerName"
    readonly property string syncType: "syncType"
    readonly property string syncs: "syncs"
    readonly property string selectiveSync: "selectiveSync"
    readonly property string fullSync: "fullSync"
    readonly property string backupsFlow: "backups"
    readonly property string finalState: "finalState"

    //property alias selectedSync: finalPage.comesFromSync

    state: computerName
    initialItem: syncsPanel

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
                    //selectedSync = true;
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
            name: backupsFlow
            StateChangeScript {
                script: {
                    //selectedSync = false;
                    if(syncsFlow.currentItem != syncsPanel) {
                        syncsFlow.replace(syncsPanel);
                    }
                    rightPanel.replace(backupsFlowPage);
                }
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepBackupsSelectFolders
            }
        },
        State {
            name: finalState
            StateChangeScript {
                script: syncsFlow.replace(finalPage);
            }
        }
    ]

    Component {
        id: finalPage

        ResumePage {
            visible: false
        }
    }

    Rectangle {
        id: syncsPanel

        width: syncsFlow.width
        height: syncsFlow.height
        visible: false

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
            Component {
                id: computerNamePage

                ComputerNamePage {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }

            Component {
                id: installationTypePage

                InstallationTypePage {}
            }

            Component {
                id: syncPage

                SyncTypePage {}
            }

            Component {
                id: fullSyncPage

                FullSyncPage {}
            }

            Component {
                id: selectiveSyncPage

                SelectiveSyncPage {}
            }

            Component{
                id: backupsFlowPage

                BackupsFlow {
                    stepLeftPanel: stepPanel
                }
            }
        }
    }
}
