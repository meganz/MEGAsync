import QtQuick 2.15

import common 1.0

import components.views 1.0

Item {
    id: root

    readonly property string syncType: "syncType"
    readonly property string fullSync: "fullSync"
    readonly property string selectiveSync: "selectiveSync"

    property bool isOnboarding: false

    signal syncsFlowMoveToFinal
    signal syncsFlowMoveToBack

    // added to avoid qml warning.
    function setInitialFocusPosition() { }

    state: syncType
        /*
           TODO: This is a temporary solution, we have to get the correct state:

        We should offer both options, if some rules apply.

        User doesn't have a sync == we offer both
        User has a sync = we offer selective

        It is simple on the surface, where we assume that if the user can do a full sync,
        we provide that option.

        syncsPanel.navInfo.fullSyncDone
                || syncsPanel.navInfo.typeSelected === SyncsType.Types.SELECTIVE_SYNC
           ? root.selectiveSync
           : root.syncType
           */

    states: [
        State {
            name: root.syncType
            StateChangeScript {
                script: {
                    view.replace(syncPageComponent);
                }
            }
            /*
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step3;
                step3Text: OnboardingStrings.syncChooseType;
                step4Text: OnboardingStrings.confirm;
            }
            */
        },
        State {
            name: root.fullSync
            StateChangeScript {
                script: {
                    //syncsPanel.navInfo.typeSelected = SyncsType.Types.FULL_SYNC;
                    view.replace(fullSyncPageComponent);
                }
            }
            /*
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step4;
                step3Text: OnboardingStrings.syncChooseType;
                step4Text: OnboardingStrings.fullSync;
            }
            */
        },
        State {
            name: root.selectiveSync
            StateChangeScript {
                script: {
                    //syncsPanel.navInfo.typeSelected = SyncsType.Types.SELECTIVE_SYNC;
                    view.replace(selectiveSyncPageComponent);
                }
            }
            /*
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step4;
                step3Text: OnboardingStrings.syncChooseType;
                step4Text: OnboardingStrings.selectiveSync;
            }
            */
        }
    ]

    StackViewBase {
        id: view

        anchors.fill: parent
        onCurrentItemChanged: {
            currentItem.setInitialFocusPosition();
        }

        Component {
            id: syncPageComponent

            SyncTypePage {
                id: syncTypePage

                footerButtons.leftSecondary.text: Strings.cancel
                footerButtons.rightSecondary.visible: root.isOnboarding
            }
        }

        Component {
            id: fullSyncPageComponent

            FullSyncPage {
                id: fullSyncPage

                footerButtons.leftSecondary.visible: root.isOnboarding
            }
        }

        Component {
            id: selectiveSyncPageComponent

            SelectiveSyncPage {
                id: selectiveSyncPage

                footerButtons.leftSecondary.visible: root.isOnboarding
            }
        }
    }

    /*
    * Navigation connections
    */

    Connections {
        id: syncTypeNavigationConnection

        target: view.currentItem
        ignoreUnknownSignals: true

        function onSyncTypeMoveToBack() {
            /*
            if(syncsPanel.navInfo.comesFromResumePage) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                root.syncsFlowMoveToFinal();
            }
            else {
                root.syncsFlowMoveToBack();
            }
            */
        }

        function onSyncTypeMoveToFullSync() {
            root.state = root.fullSync;
        }

        function onSyncTypeMoveToSelectiveSync() {
            root.state = root.selectiveSync;
        }
    }

    Connections {
        id: selectiveSyncNavigationConnection

        target: view.currentItem
        ignoreUnknownSignals: true

        function onSelectiveSyncMoveToBack() {
            /*
            if(syncsPanel.navInfo.comesFromResumePage && syncsPanel.navInfo.syncDone) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                root.syncsFlowMoveToFinal();
            }
            else {*/
                root.state = root.syncType;
            //}
        }

        function onSelectiveSyncMoveToSuccess() {
            //syncsPanel.navInfo.selectiveSyncDone = true;
            root.syncsFlowMoveToFinal();
        }
    }

    Connections {
        id: fullSyncNavigationConnection

        target: view.currentItem
        ignoreUnknownSignals: true

        function onFullSyncMoveToBack() {
            /*
            if(syncsPanel.navInfo.comesFromResumePage && syncsPanel.navInfo.syncDone) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                root.syncsFlowMoveToFinal();
            }
            else {*/
                root.state = root.syncType;
            //}
        }

        function onFullSyncMoveToSuccess() {
           // syncsPanel.navInfo.fullSyncDone = true;
            root.syncsFlowMoveToFinal();
        }
    }
}
