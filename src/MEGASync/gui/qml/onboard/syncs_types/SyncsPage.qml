import QtQuick 2.15

import common 1.0

import onboard 1.0

import SyncsComponents 1.0

import syncs 1.0 as Syncs

SyncsFlow {
    id: root

    required property StepPanel stepPanelRef
    required property NavigationInfo navInfoRef

    addSyncPageComponent: addSyncPageComponentItem
    confirmSyncsPageComponent: confirmSyncsPageComponentItem

    signal syncsFlowMoveToFinal(int syncType)
    signal syncsFlowMoveToBack()

    Item {
        id: stepPanelStateWrapper

        readonly property string addSyncPage: "addSyncPage"

        states: [
            State {
                name: stepPanelStateWrapper.addSyncPage
                PropertyChanges {
                    target: root.stepPanelRef;
                    state: root.stepPanelRef.step4;
                    step3Text: Syncs.SyncsStrings.selectFolders;
                    step4Text: Syncs.SyncsStrings.confirm;
                }
            }
        ]
    }

    onStateChanged: {
        switch(root.state) {
            case root.addSync:
                //navInfoRef.typeSelected = Constants.SyncType.SELECTIVE_SYNC;
                stepPanelStateWrapper.state = stepPanelStateWrapper.addSyncPage;
                break;

            case root.confirmSyncs:
                //navInfoRef.typeSelected = Constants.SyncType.SELECTIVE_SYNC;
                stepPanelStateWrapper.state = stepPanelStateWrapper.confirmSyncPage;
                break;

            default:
                console.warn("BackupsPage: state does not exist -> " + root.state);
                break;
        }
    }

    Component {
        id: addSyncPageComponentItem

        AddSyncPage {
            id: addSyncPage

            footerButtons.rightSecondary.visible: true
            footerButtons.leftSecondary.visible: false
            footerButtons.leftPrimary.text: Strings.skip
            footerButtons.leftPrimary.onClicked: {
                window.close();
            }

            onMoveBack: {
                root.syncsFlowMoveToBack(false);
            }

            onMoveNext: {
                root.state = root.confirmSyncs
            }
        }
    }

    Component {
        id: confirmSyncsPageComponentItem

        ConfirmSyncsPage {
            id: confirmSyncsPage

            onSelectiveSyncMoveToSuccess: {
                root.syncsFlowMoveToFinal(Constants.SyncType.SELECTIVE_SYNC);
            }

            onFullSyncMoveToSuccess: {
                root.syncsFlowMoveToFinal(Constants.SyncType.FULL_SYNC);
            }
        }
    }

}
