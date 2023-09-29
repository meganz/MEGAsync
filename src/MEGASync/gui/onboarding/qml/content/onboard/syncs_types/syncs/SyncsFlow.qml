// System
import QtQuick 2.12
import QtQuick.Controls 2.12

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0
import Onboard.Syncs_types.Left_panel 1.0

// C++
import BackupsProxyModel 1.0

StackView {
    id: syncsFlow

    readonly property string syncType: "syncType"
    readonly property string fullSync: "full"
    readonly property string selectiveSync: "selective"

    state: syncsPanel.fullSyncDone || syncsPanel.typeSelected === SyncsType.Types.SelectiveSync
           ? selectiveSync
           : syncType

    states: [
        State {
            name: syncType
            StateChangeScript {
                script: syncsFlow.replace(syncPage);
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
                    typeSelected = SyncsType.Types.FullSync;
                    syncsFlow.replace(fullSyncPage);
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
                    typeSelected = SyncsType.Types.SelectiveSync;
                    syncsFlow.replace(selectiveSyncPage);
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
