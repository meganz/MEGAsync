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
    signal syncsFlowMoveToFinalError()
    signal syncsFlowMoveToBack()

    onStateChanged: {
        switch(root.state) {
            case root.addSync:
                navInfoRef.typeSelected = Constants.SyncType.SYNC;

                root.stepPanelRef.state = root.stepPanelRef.step3;
                root.stepPanelRef.step3Text = Syncs.SyncsStrings.selectFolders;
                root.stepPanelRef.step4Text = Syncs.SyncsStrings.confirm;
                break;

            case root.confirmSyncs:
                root.stepPanelRef.state = root.stepPanelRef.step4;
                break;

            default:
                console.warn("SyncsPage: state " + root.state + " is not being handled.");
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

            onSyncSetupSucceed: (isFullSync) => {
                if (isFullSync) {
                    root.syncsFlowMoveToFinal(Constants.SyncType.FULL_SYNC);
                }
                else {
                    root.syncsFlowMoveToFinal(Constants.SyncType.SELECTIVE_SYNC);
                }
            }

            onSyncSetupFailed: {
                root.syncsFlowMoveToFinalError();
            }

            onMoveBack: {
                root.state = root.addSync;
            }
        }
    }

}
