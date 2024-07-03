import QtQuick 2.15

import common 1.0

import components.steps 1.0

import Syncs 1.0

SyncsFlow {
    id: root

    required property StepPanel stepPanelRef
    required property var syncsContentItemRef

    syncPageComponent: syncPageComponentItem
    fullSyncPageComponent: fullSyncPageComponentItem
    selectiveSyncPageComponent: selectiveSyncPageComponentItem

    Item {
        id: stepPanelStateWrapper

        readonly property string selectSyncTypePage: "selectSyncTypePage"
        readonly property string selectiveSyncPage: "selectiveSyncPage"
        readonly property string fullSyncPage: "fullSyncPage"

        states: [
            State {
                name: stepPanelStateWrapper.selectSyncTypePage
                PropertyChanges {
                    target: root.stepPanelRef
                    state: root.stepPanelRef.step1
                    step2String: SyncsStrings.sync
                }
            },
            State {
                name: stepPanelStateWrapper.selectiveSyncPage
                PropertyChanges {
                    target: root.stepPanelRef
                    state: root.stepPanelRef.step2
                    step2String: SyncsStrings.selectiveSync
                }
            },
            State {
                name: stepPanelStateWrapper.fullSyncPage
                PropertyChanges {
                    target: root.stepPanelRef
                    state: root.stepPanelRef.step2
                    step2String: SyncsStrings.fullSync
                }
            }
        ]
    }

    onSyncsFlowMoveToFinal: (success) => {
        if (success) {
            syncsContentItemRef.state = syncsContentItemRef.resume;
        }
    }

    onStateChanged: {
        switch(root.state) {
            case root.syncType:
                stepPanelStateWrapper.state = stepPanelStateWrapper.selectSyncTypePage;
                break;
            case root.fullSync:
                stepPanelStateWrapper.state = stepPanelStateWrapper.fullSyncPage;
                break;
            case root.selectiveSync:
                stepPanelStateWrapper.state = stepPanelStateWrapper.selectiveSyncPage;
                break;
            default:
                console.warn("SyncPage: state does not exist -> " + root.state);
                break;
        }
    }

    Component {
       id: syncPageComponentItem

       SyncTypePage {
           id: syncTypePage

           footerButtons.leftPrimary.visible: false
           footerButtons.leftSecondary.visible: false
           footerButtons.rightSecondary.text: Strings.cancel
           footerButtons.rightSecondary.visible: true

           fullSyncButton.width: 280
           fullSyncButton.imageSource: Images.syncTypeFull
           fullSyncButton.imageSourceSize: Qt.size(256, 100)

           selectiveSyncButton.width: 280
           selectiveSyncButton.imageSource: Images.syncTypeSelective
           selectiveSyncButton.imageSourceSize: Qt.size(256, 100)

           onSyncTypeMoveToBack: {
               window.close();
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

            isOnboarding: false
            footerButtons.leftPrimary.visible: false
            footerButtons.leftSecondary.visible: true
            footerButtons.leftSecondary.text: Strings.setExclusions

            onFullSyncMoveToBack: {
                root.state = root.syncType;
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

            isOnboarding: false
            footerButtons.leftPrimary.visible: false
            footerButtons.leftSecondary {
                text: Strings.setExclusions
                visible: true
            }

            footerButtons.rightSecondary.text: (root.sync.syncStatus === root.sync.SyncStatusCode.NONE) ? Strings.previous : Strings.cancel

            onSelectiveSyncMoveToBack: {
                if(root.sync.syncStatus === root.sync.SyncStatusCode.NONE) {
                    root.state = root.syncType;
                }
                else {
                    window.close();
                }
            }

            onSelectiveSyncMoveToSuccess: {
                root.sync.syncStatus = root.sync.SyncStatusCode.SELECTIVE;
                root.syncsFlowMoveToFinal(Constants.SyncType.SELECTIVE_SYNC);
            }
        }
    }

}
