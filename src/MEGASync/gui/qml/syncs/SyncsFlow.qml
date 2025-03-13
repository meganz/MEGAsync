import QtQuick 2.15

import common 1.0

import components.views 1.0

Item {
    id: root

    required property Component selectiveSyncPageComponent

    readonly property string selectiveSync: "selectiveSync"

    property bool isOnboarding: false

    signal syncsFlowMoveToFinal(int syncType)
    signal syncsFlowMoveToBack()

    // added to avoid qml warning.
    function setInitialFocusPosition() { }

    state: root.selectiveSync

    states: [
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
