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

    onSyncsFlowMoveToFinal: {
        syncsContentItemRef.state = syncsContentItemRef.resume;
    }

    Component {
        id: selectiveSyncPageComponentItem

        SelectiveSyncPage {
            id: selectiveSyncPage

            footerButtons {
                leftPrimary.visible: false
                leftSecondary {
                    text: Strings.setExclusions
                    visible: localFolderChooser.choosenPath.length !== 0
                }
            }

            onSelectiveSyncMoveToSuccess: {
                root.syncsFlowMoveToFinal(Constants.SyncType.SELECTIVE_SYNC);
            }

            onFullSyncMoveToSuccess: {
                root.syncsFlowMoveToFinal(Constants.SyncType.FULL_SYNC);
            }
        }
    }

}
