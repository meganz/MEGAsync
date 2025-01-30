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
    state: root.selectiveSync

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
                    step2String: SyncsStrings.confirm
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

            footerButtons {
                leftPrimary.visible: false
                leftSecondary.visible: false
                rightSecondary.text: Strings.cancel
                rightSecondary.visible: true
            }

            fullSyncButton {
                width: 280
                imageSource: Images.syncTypeFull
                imageSourceSize: Qt.size(256, 100)
            }

            selectiveSyncButton {
                width: 280
                imageSource: Images.syncTypeSelective
                imageSourceSize: Qt.size(256, 100)
            }

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
            footerButtons {
                leftPrimary.visible: false
                leftSecondary {
                    text: Strings.setExclusions
                    visible: localFolderChooser.choosenPath.length !== 0
                }
            }

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
            footerButtons {
                leftPrimary.visible: false
                leftSecondary {
                    text: Strings.setExclusions
                    visible: localFolderChooser.choosenPath.length !== 0
                }
                rightSecondary.text: (root.sync.syncStatus === root.sync.SyncStatusCode.NONE) ? Strings.previous : Strings.cancel
            }

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

            onFullSyncMoveToSuccess: {
                root.sync.syncStatus = root.sync.SyncStatusCode.FULL;
                root.syncsFlowMoveToFinal(Constants.SyncType.FULL_SYNC);
            }
        }
    }

}
