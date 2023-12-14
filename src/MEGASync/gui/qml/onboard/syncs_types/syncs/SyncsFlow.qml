import QtQuick 2.15
import QtQuick.Controls 2.15

import onboard 1.0
import onboard.syncs_types 1.0
import onboard.syncs_types.left_panel 1.0

import BackupsProxyModel 1.0

Item {
    id: root

    signal syncsFlowMoveToFinal
    signal syncsFlowMoveToBack

    readonly property string syncType: "syncType"
    readonly property string fullSync: "full"
    readonly property string selectiveSync: "selective"

    // added to avoid qml warning.
    function setInitialFocusPosition() {
    }

    state: syncsPanel.navInfo.fullSyncDone
                || syncsPanel.navInfo.typeSelected === SyncsType.Types.SELECTIVE_SYNC
           ? selectiveSync
           : syncType

    states: [
        State {
            name: syncType
            StateChangeScript {
                script: {
                    view.replace(syncPage);
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
            name: fullSync
            StateChangeScript {
                script: {
                    syncsPanel.navInfo.typeSelected = SyncsType.Types.FULL_SYNC;
                    view.replace(fullSyncPage);
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step4;
                step3Text: OnboardingStrings.syncChooseType;
                step4Text: OnboardingStrings.fullSync;
            }
        },
        State {
            name: selectiveSync
            StateChangeScript {
                script: {
                    syncsPanel.navInfo.typeSelected = SyncsType.Types.SELECTIVE_SYNC;
                    view.replace(selectiveSyncPage);
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step4;
                step3Text: OnboardingStrings.syncChooseType;
                step4Text: OnboardingStrings.selectiveSync;
            }
        }
    ]

    StackViewBase {
        id: view
        anchors.fill: parent

        onCurrentItemChanged: {
            currentItem.setInitialFocusPosition();
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
    }

    /*
    * Navigation connections
    */
    Connections {
        id: syncTypeNavigationConnection
        target: view.currentItem
        ignoreUnknownSignals: true

        function onSyncTypeMoveToBack() {
            if(syncsPanel.navInfo.comesFromResumePage) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                root.syncsFlowMoveToFinal()
            } else {
                root.syncsFlowMoveToBack()
            }
        }

        function onSyncTypeMoveToFullSync() {
            root.state = root.fullSync
        }

        function onSyncTypeMoveToSelectiveSync() {
            root.state = root.selectiveSync
        }
    }

    Connections {
        id: selectiveSyncNavigationConnection
        target: view.currentItem
        ignoreUnknownSignals: true

        function onSelectiveSyncMoveToBack() {
            if(syncsPanel.navInfo.comesFromResumePage && syncsPanel.navInfo.syncDone) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                root.syncsFlowMoveToFinal()
            }
            else {
                root.state = root.syncType
            }
        }

        function onSelectiveSyncMoveToSuccess() {
            syncsPanel.navInfo.selectiveSyncDone = true
            root.syncsFlowMoveToFinal()
        }
    }

    Connections {
        id: fullSyncNavigationConnection
        target: view.currentItem
        ignoreUnknownSignals: true

        function onFullSyncMoveToBack() {
            if(syncsPanel.navInfo.comesFromResumePage && syncsPanel.navInfo.syncDone) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                root.syncsFlowMoveToFinal()

            } else {
                root.state = root.syncType
            }
        }

        function onFullSyncMoveToSuccess() {
            syncsPanel.navInfo.fullSyncDone = true
            root.syncsFlowMoveToFinal()
        }
    }
}
