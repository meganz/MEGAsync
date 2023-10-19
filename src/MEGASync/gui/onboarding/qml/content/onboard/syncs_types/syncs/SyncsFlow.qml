// System
import QtQuick 2.12
import QtQuick.Controls 2.12

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0
import Onboard.Syncs_types.Left_panel 1.0

// C++
import BackupsProxyModel 1.0

Item {
    id: root

    signal moveToFinal
    signal moveToSyncType

    readonly property string syncType: "syncType"
    readonly property string fullSync: "full"
    readonly property string selectiveSync: "selective"

    state: syncsPanel.navInfo.fullSyncDone || syncsPanel.navInfo.typeSelected === SyncsType.Types.SelectiveSync
           ? selectiveSync
           : syncType

    states: [
        State {
            name: syncType
            StateChangeScript {
                script: {
                    syncTypeNavigationConnection.enabled = true
                    selectiveSyncNavigationConnection.enabled = false
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
                    syncTypeNavigationConnection.enabled = false
                    selectiveSyncNavigationConnection.enabled = false
                    syncsPanel.navInfo.typeSelected = SyncsType.Types.FullSync;
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
                    syncTypeNavigationConnection.enabled = false
                    selectiveSyncNavigationConnection.enabled = true
                    syncsPanel.navInfo.typeSelected = SyncsType.Types.SelectiveSync;
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

    StackView {
        id: view
        anchors.fill: parent

        replaceEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to:1
                duration: 100
                easing.type: Easing.OutQuad
            }
        }
        replaceExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to:0
                duration: 100
                easing.type: Easing.InQuad
            }
        }
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

    /*
    * Navigation connections
    */
    Connections {
        id: syncTypeNavigationConnection
        target: view.currentItem
        ignoreUnknownSignals: true
        enabled: false

        function onMoveToBack() {
            if(syncsPanel.navInfo.comesFromResumePage) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                root.moveToFinal()
            } else {
                root.moveToSyncType()
            }
        }

        function onMoveToFullSync()
        {
            root.state = root.fullSync
        }

        function onMoveToSelectiveSync()
        {
            root.state = root.selectiveSync
        }
    }

    Connections {
        id: selectiveSyncNavigationConnection
        target: view.currentItem
        ignoreUnknownSignals: true
        enabled: false

        function onMoveToBack()
        {
            if(syncsPanel.navInfo.comesFromResumePage && syncsPanel.navInfo.syncDone) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                root.moveToFinal()
            }
            else {
                root.state = root.syncType;
            }
        }

        function onMoveToFinal()
        {
            root.moveToFinal()
        }
    }
}
