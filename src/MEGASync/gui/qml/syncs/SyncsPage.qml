import QtQuick 2.15

import common 1.0

import components.steps 1.0

SyncsFlow {
    id: root

    required property StepPanel stepPanelRef
    syncPageComponent: syncPageComponent

    Component {
        id: syncPageComponent

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
}
