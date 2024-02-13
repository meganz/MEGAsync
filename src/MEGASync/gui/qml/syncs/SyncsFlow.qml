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
    signal syncsFlowMoveToBack(bool fromSelectType)

    // added to avoid qml warning.
    function setInitialFocusPosition() { }

    state: syncItem.syncStatus !== syncItem.SyncStatusCode.NONE
           ? root.selectiveSync
           : root.syncType

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

    Syncs {
        id: syncItem
    }

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

        Component {
            id: fullSyncPageComponent

            FullSyncPage {
                id: fullSyncPage

                isOnboardingRef: root.isOnboarding

                footerButtons.leftSecondary {
                    text: root.isOnboarding ? Strings.skip : Strings.cancel
                    visible: root.isOnboarding
                }

                onFullSyncMoveToBack: {
                    // If it is the standalone window, then move to the sync type page
                    // Otherwise, delegate the state control to MainFlow using the signal
                    if(!isOnboarding && syncItem.syncStatus === syncItem.SyncStatusCode.NONE) {
                        root.state = root.syncType;
                    }
                    else {
                        root.syncsFlowMoveToBack(false);
                    }
                }

                onFullSyncMoveToSuccess: {
                    syncItem.syncStatus = syncItem.SyncStatusCode.FULL;
                    root.syncsFlowMoveToFinal(Constants.SyncType.FULL_SYNC);
                }
            }
        }

        Component {
            id: selectiveSyncPageComponent

            SelectiveSyncPage {
                id: selectiveSyncPage

                isOnboardingRef: root.isOnboarding

                footerButtons.rightSecondary.visible: root.isOnboarding
                                                        || (!root.isOnboarding && syncItem.syncStatus === syncItem.SyncStatusCode.NONE
                                                                && syncsComponentAccess.remoteFolder === "")
                footerButtons.leftSecondary {
                    text: root.isOnboarding ? Strings.skip : Strings.cancel
                    visible: root.isOnboarding
                                || (!root.isOnboarding && syncItem.syncStatus !== syncItem.SyncStatusCode.NONE)
                                || syncsComponentAccess.remoteFolder !== ""
                }

                onSelectiveSyncMoveToBack: {
                    // If it is the standalone window, then move to the sync type page
                    // Otherwise, delegate the state control to MainFlow using the signal
                    if(!isOnboarding && syncItem.syncStatus === syncItem.SyncStatusCode.NONE) {
                        root.state = root.syncType;
                    }
                    else {
                        root.syncsFlowMoveToBack(false);
                    }
                }

                onSelectiveSyncMoveToSuccess: {
                    syncItem.syncStatus = syncItem.SyncStatusCode.SELECTIVE;
                    root.syncsFlowMoveToFinal(Constants.SyncType.SELECTIVE_SYNC);
                }
            }
        }

    } // StackViewBase: view

    Connections {
        id: syncsComponentAccessConnection

        target: isOnboarding ? null : syncsComponentAccess
        enabled: !isOnboarding
        ignoreUnknownSignals: true

        function onRemoteFolderChanged() {
            if(syncsComponentAccess.remoteFolder !== "") {
                root.state = root.selectiveSync;
            }
        }
    }

}
