import QtQuick 2.15

import common 1.0

import components.steps 1.0

SyncsFlow {
    id: root

    required property StepPanel stepPanelRef

    syncPageComponent: Component {
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
}
