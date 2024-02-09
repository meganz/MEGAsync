import QtQuick 2.15

import common 1.0

import syncs 1.0

import onboard 1.0
import onboard.syncs_types.left_panel 1.0

SyncsFlow {
    id: root

    required property StepPanel stepPanelRef

    isOnboarding: true

    state: syncsPanel.navInfo.fullSyncDone
                || syncsPanel.navInfo.typeSelected === Constants.SyncType.SELECTIVE_SYNC
           ? root.selectiveSync
           : root.syncType

    Item {
        id: stepPanelStateWrapper

        readonly property string syncTypePage: "syncTypePage"
        readonly property string selectiveSyncPage: "selectiveSyncPage"
        readonly property string fullSyncPage: "fullSyncPage"

        states: [
            State {
                name: stepPanelStateWrapper.syncTypePage
                PropertyChanges {
                    target: root.stepPanelRef;
                    state: root.stepPanelRef.step3;
                    step3Text: OnboardingStrings.syncChooseType;
                    step4Text: OnboardingStrings.confirm;
                }
            },
            State {
                name: stepPanelStateWrapper.selectiveSyncPage
                PropertyChanges {
                    target: root.stepPanelRef;
                    state: root.stepPanelRef.step4;
                    step3Text: OnboardingStrings.syncChooseType;
                    step4Text: SyncsStrings.selectiveSync;
                }
            },
            State {
                name: stepPanelStateWrapper.fullSyncPage
                PropertyChanges {
                    target: root.stepPanelRef;
                    state: root.stepPanelRef.step4;
                    step3Text: OnboardingStrings.syncChooseType;
                    step4Text: SyncsStrings.fullSync;
                }
            }
        ]
    }

    onStateChanged: {
        switch(root.state) {
            case root.syncType:
                stepPanelStateWrapper.state = stepPanelStateWrapper.syncTypePage;
                break;
            case root.selectiveSync:
                syncsPanel.navInfo.typeSelected = Constants.SyncType.SELECTIVE_SYNC;
                stepPanelStateWrapper.state = stepPanelStateWrapper.selectiveSyncPage;
                break;
            case root.fullSync:
                syncsPanel.navInfo.typeSelected = Constants.SyncType.FULL_SYNC;
                stepPanelStateWrapper.state = stepPanelStateWrapper.fullSyncPage;
                break;
            default:
                console.warn("BackupsPage: state does not exist -> " + root.state);
                break;
        }
    }

}
