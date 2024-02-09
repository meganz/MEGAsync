import QtQuick 2.15

import common 1.0

import components.views 1.0

import Syncs 1.0

Item {
    id: root

    readonly property string syncType: "syncType"
    readonly property string fullSync: "fullSync"
    readonly property string selectiveSync: "selectiveSync"

    property bool isOnboarding: false

    signal syncsFlowMoveToFinal(int syncType)
    signal syncsFlowMoveToBack

    // added to avoid qml warning.
    function setInitialFocusPosition() { }

    state: syncItem.syncStatus === undefined || syncItem.syncStatus === syncItem.SyncStatusCode.NONE
           ? root.syncType
           : root.selectiveSync

    states: [
        State {
            name: root.syncType
            StateChangeScript {
                script: {
                    view.replace(syncPageComponent);
                }
            }
        },
        State {
            name: root.fullSync
            StateChangeScript {
                script: {
                    view.replace(fullSyncPageComponent);
                }
            }
        },
        State {
            name: root.selectiveSync
            StateChangeScript {
                script: {
                    view.replace(selectiveSyncPageComponent);
                }
            }
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

                footerButtons.leftSecondary.text: root.isOnboarding ? Strings.skip : Strings.cancel
                footerButtons.rightSecondary.visible: root.isOnboarding
            }
        }

        Component {
            id: fullSyncPageComponent

            FullSyncPage {
                id: fullSyncPage

                footerButtons.leftSecondary {
                    text: root.isOnboarding ? Strings.skip : Strings.cancel
                    visible: root.isOnboarding
                }
            }
        }

        Component {
            id: selectiveSyncPageComponent

            SelectiveSyncPage {
                id: selectiveSyncPage

                footerButtons.rightSecondary.visible: root.isOnboarding
                                                        || (!root.isOnboarding
                                                                && syncItem.syncStatus === syncItem.SyncStatusCode.NONE)
                footerButtons.leftSecondary {
                    text: root.isOnboarding ? Strings.skip : Strings.cancel
                    visible: root.isOnboarding
                                || (!root.isOnboarding
                                        && syncItem.syncStatus !== syncItem.SyncStatusCode.NONE)
                }
            }
        }
    }

    Syncs {
        id: syncItem
    }

    /*
    * Navigation connections
    */

    Connections {
        id: syncTypeNavigationConnection

        target: view.currentItem
        ignoreUnknownSignals: true

        function onSyncTypeMoveToBack() {
            root.syncsFlowMoveToBack();
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
                //root.state = root.syncType;
            //}
            if(syncItem.syncStatus === syncItem.SyncStatusCode.NONE) {
                root.state = root.syncType;
            }
            else {
                root.syncsFlowMoveToBack();
            }
        }

        function onSelectiveSyncMoveToSuccess() {
            //syncsPanel.navInfo.selectiveSyncDone = true;
            syncItem.syncStatus = syncItem.SyncStatusCode.SELECTIVE;
            root.syncsFlowMoveToFinal(Constants.SyncType.SELECTIVE_SYNC);
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
                //root.state = root.syncType;
            //}

            if(syncItem.syncStatus === syncItem.SyncStatusCode.NONE) {
                root.state = root.syncType;
            }
            else {
                root.syncsFlowMoveToBack();
            }
        }

        function onFullSyncMoveToSuccess() {
           // syncsPanel.navInfo.fullSyncDone = true;
            syncItem.syncStatus = syncItem.SyncStatusCode.FULL;
            root.syncsFlowMoveToFinal(Constants.SyncType.FULL_SYNC);
        }
    }
}
