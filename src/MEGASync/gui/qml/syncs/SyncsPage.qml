import QtQuick 2.15

import common 1.0

import components.steps 1.0

import Syncs 1.0

SyncsFlow {
    id: root

    required property StepPanel stepPanelRef
    required property var syncsContentItemRef

    selectiveSyncPageComponent: selectiveSyncPageComponentItem
    state: root.selectiveSync

    onSyncsFlowMoveToFinal: (success) => {
        if (success) {
            syncsContentItemRef.state = syncsContentItemRef.resume;
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
