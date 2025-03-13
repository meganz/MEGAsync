import QtQuick 2.15

import common 1.0

import syncs 1.0
import onboard 1.0

import SyncsComponents 1.0

SyncsFlow {
    id: root

    required property StepPanel stepPanelRef
    required property NavigationInfo navInfoRef

    selectiveSyncPageComponent: selectiveSyncPageComponentItem

    isOnboarding: true

    state: root.selectiveSync

    Item {
        id: stepPanelStateWrapper

        readonly property string selectiveSyncPage: "selectiveSyncPage"

        states: [
            State {
                name: stepPanelStateWrapper.selectiveSyncPage
                PropertyChanges {
                    target: root.stepPanelRef;
                    state: root.stepPanelRef.step4;
                    step3Text: SyncsStrings.selectFolders;
                    step4Text: OnboardingStrings.confirm;
                }
            }
        ]
    }

    onStateChanged: {
        switch(root.state) {
            case root.selectiveSync:
                navInfoRef.typeSelected = Constants.SyncType.SELECTIVE_SYNC;
                stepPanelStateWrapper.state = stepPanelStateWrapper.selectiveSyncPage;
                break;
            default:
                console.warn("BackupsPage: state does not exist -> " + root.state);
                break;
        }
    }

    Component {
        id: selectiveSyncPageComponentItem

        SelectiveSyncPage {
            id: selectiveSyncPage

            isOnboarding: true
            footerButtons.rightSecondary.visible: true
            footerButtons.leftSecondary.visible: false
            footerButtons.leftPrimary.text: Strings.skip
            footerButtons.leftPrimary.onClicked: {
                window.close();
            }

            onSelectiveSyncMoveToBack: {
                root.syncsFlowMoveToBack(false);
            }

            onSelectiveSyncMoveToSuccess: {
                syncs.syncStatus = SyncStatusCode.SELECTIVE;
                root.syncsFlowMoveToFinal(Constants.SyncType.SELECTIVE_SYNC);
            }

            onFullSyncMoveToSuccess: {
                syncs.syncStatus = SyncStatusCode.FULL;
                root.syncsFlowMoveToFinal(Constants.SyncType.FULL_SYNC);
            }
        }
    }

}

