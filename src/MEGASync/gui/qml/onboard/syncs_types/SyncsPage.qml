import QtQuick 2.15

import common 1.0

import syncs 1.0

import onboard 1.0

import Syncs 1.0

SyncsFlow {
    id: root

    required property StepPanel stepPanelRef
    required property NavigationInfo navInfoRef

    syncPageComponent: syncPageComponentItem
    fullSyncPageComponent: fullSyncPageComponentItem
    selectiveSyncPageComponent: selectiveSyncPageComponentItem

    isOnboarding: true

    state: navInfoRef.fullSyncDone || navInfoRef.typeSelected === Constants.SyncType.SELECTIVE_SYNC
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
                navInfoRef.typeSelected = Constants.SyncType.SELECTIVE_SYNC;
                stepPanelStateWrapper.state = stepPanelStateWrapper.selectiveSyncPage;
                break;
            case root.fullSync:
                navInfoRef.typeSelected = Constants.SyncType.FULL_SYNC;
                stepPanelStateWrapper.state = stepPanelStateWrapper.fullSyncPage;
                break;
            default:
                console.warn("BackupsPage: state does not exist -> " + root.state);
                break;
        }
    }

    Component {
        id: syncPageComponentItem

        SyncTypePage {
            id: syncTypePage

            footerButtons {
                leftPrimary.visible: true
                leftPrimary.text: Strings.skip
                leftPrimary.onClicked: {
                    window.close();
                }

                leftSecondary.visible: false
                rightSecondary.visible: true
            }

            onSyncTypeMoveToBack: {
                root.syncsFlowMoveToBack(true);
            }

            onSyncTypeMoveToFullSync: {
                root.state = root.fullSync;
            }

            onSyncTypeMoveToSelectiveSync: {
                root.state = root.selectiveSync;
            }
        }
    }

    Component {
        id: fullSyncPageComponentItem

        FullSyncPage {
            id: fullSyncPage

            isOnboarding: true

            footerButtons.leftPrimary.text: Strings.skip
            footerButtons.leftPrimary.onClicked: {
                window.close();
            }

            onFullSyncMoveToBack: {
                 root.syncsFlowMoveToBack(false);
            }

            onFullSyncMoveToSuccess: {
                root.sync.syncStatus = root.sync.SyncStatusCode.FULL;
                root.syncsFlowMoveToFinal(Constants.SyncType.FULL_SYNC);
            }
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
                root.sync.syncStatus = root.sync.SyncStatusCode.SELECTIVE;
                root.syncsFlowMoveToFinal(Constants.SyncType.SELECTIVE_SYNC);
            }
        }
    }

}

