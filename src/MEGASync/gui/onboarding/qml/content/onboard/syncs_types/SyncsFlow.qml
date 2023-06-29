// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
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

    readonly property int stepPanelWidth: 304
    readonly property int contentMargin: 48
    readonly property int contentHeight: 464
    readonly property int lineWidth: 2

    property int lastTypeSelected: SyncsType.Types.None

    state: computerName
    initialItem: syncsPanel

    states: [
        State {
            name: computerName
            StateChangeScript {
                script: rightPanel.replace(computerNamePage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step1ComputerName;
            }
        },
        State {
            name: syncType
            StateChangeScript {
                script: rightPanel.replace(installationTypePage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step2InstallationType;
            }
        },
        State {
            name: syncs
            StateChangeScript {
                script: {
                    lastTypeSelected = SyncsType.Types.Sync;
                    rightPanel.replace(syncPage);
                    if(syncsFlow.currentItem != syncsPanel) {
                        syncsFlow.replace(syncsPanel);
                    }
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step3;
                step3Text: OnboardingStrings.syncChooseType;
                step4Text: OnboardingStrings.confirm;
            }
        },
        State {
            name: selectiveSync
            StateChangeScript {
                script: rightPanel.replace(selectiveSyncPage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step4;
                step3Text: OnboardingStrings.syncChooseType;
                step4Text: OnboardingStrings.selectiveSync;
            }
        },
        State {
            name: fullSync
            StateChangeScript {
                script: rightPanel.replace(fullSyncPage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step4;
                step3Text: OnboardingStrings.syncChooseType;
                step4Text: OnboardingStrings.fullSync;
            }
        },
        State {
            name: backupsFlow
            StateChangeScript {
                script: {
                    lastTypeSelected = SyncsType.Types.Backup;
                    if(syncsFlow.currentItem != syncsPanel) {
                        syncsFlow.replace(syncsPanel);
                    }
                    rightPanel.replace(backupsFlowPage);
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step3;
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
        color: Styles.surface1

        Rectangle {
            id: leftPanel

            width: stepPanelWidth + lineWidth
            height: parent.height
            color: Styles.surface1
            z: 2

            StepPanel {
                id: stepPanel

                anchors.fill: parent
                anchors.topMargin: contentMargin
                anchors.bottomMargin: contentMargin
                anchors.leftMargin: contentMargin
            }

            Rectangle {
                id: separatorLine

                width: lineWidth
                radius: lineWidth
                color: Styles.borderDisabled
                height: contentHeight
                anchors.left: leftPanel.right
                anchors.top: parent.top
                anchors.topMargin: contentMargin
            }
        }

        StackView {
            id: rightPanel

            anchors {
                left: leftPanel.right
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

                BackupsFlow {}
            }
        }
    }
}
