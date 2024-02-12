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

    state: (syncItem.syncStatus === undefined || syncItem.syncStatus === syncItem.SyncStatusCode.NONE)
                && syncsComponentAccess.remoteFolder === ""
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
                    root.syncsFlowMoveToBack();
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

                footerButtons.leftSecondary {
                    text: root.isOnboarding ? Strings.skip : Strings.cancel
                    visible: root.isOnboarding
                }

                onFullSyncMoveToBack: {
                    if(syncItem.syncStatus === syncItem.SyncStatusCode.NONE) {
                        root.state = root.syncType;
                    }
                    else {
                        root.syncsFlowMoveToBack();
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

                function buttonVisible() {
                    return root.isOnboarding
                                || (!root.isOnboarding && syncItem.syncStatus === syncItem.SyncStatusCode.NONE)
                }

                footerButtons.rightSecondary.visible: selectiveSyncPage.buttonVisible()
                footerButtons.leftSecondary {
                    text: root.isOnboarding ? Strings.skip : Strings.cancel
                    visible: selectiveSyncPage.buttonVisible()
                }

                onSelectiveSyncMoveToBack: {
                    if(syncItem.syncStatus === syncItem.SyncStatusCode.NONE) {
                        root.state = root.syncType;
                    }
                    else {
                        root.syncsFlowMoveToBack();
                    }
                }

                onSelectiveSyncMoveToSuccess: {
                    syncItem.syncStatus = syncItem.SyncStatusCode.SELECTIVE;
                    root.syncsFlowMoveToFinal(Constants.SyncType.SELECTIVE_SYNC);
                }
            }
        }

    } // StackViewBase: view

}
