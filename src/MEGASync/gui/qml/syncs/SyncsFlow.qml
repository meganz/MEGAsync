import QtQuick 2.15

import common 1.0

import components.views 1.0

import Syncs 1.0

Item {
    id: root

    property alias sync: syncItem

    required property Component syncPageComponent
    required property Component fullSyncPageComponent
    required property Component selectiveSyncPageComponent

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
    }

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
