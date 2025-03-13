import QtQuick 2.15

import common 1.0

import components.steps 1.0

import syncs 1.0
import SyncsComponents 1.0

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
                rightSecondary.text: (syncsData.syncStatus === SyncsUtils.SyncStatusCode.NONE) ? Strings.previous : Strings.cancel
            }

            onSelectiveSyncMoveToBack: {
                if(syncsData.syncStatus === SyncsUtils.SyncStatusCode.NONE) {
                    root.state = root.syncType;
                }
                else {
                    window.close();
                }
            }

            onSelectiveSyncMoveToSuccess: {
                syncsComponentAccess.syncStatus = SyncsUtils.SyncStatusCode.SELECTIVE;
                root.syncsFlowMoveToFinal(Constants.SyncType.SELECTIVE_SYNC);
            }

            onFullSyncMoveToSuccess: {
                syncsComponentAccess.syncStatus = SyncsUtils.SyncStatusCode.FULL;
                root.syncsFlowMoveToFinal(Constants.SyncType.FULL_SYNC);
            }
        }
    }

}
