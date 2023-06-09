// System
import QtQuick 2.12
import QtQuick.Controls 2.12

// Local
import Onboard.Syncs_types.Left_panel 1.0

// C++
import BackupsProxyModel 1.0

StackView {
    id: syncsFlow


    readonly property string syncType: "syncType"
    readonly property string fullSync: "full"
    readonly property string selectiveSync: "selective"

    state: syncType

    states: [
        State {
            name: syncType
            StateChangeScript {
                script: syncsFlow.replace(syncPage);
            }
        },
        State {
            name: fullSync
            StateChangeScript {
                script: syncsFlow.replace(fullSyncPage);
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepSyncFolder
            }
        },
        State {
            name: selectiveSync
            StateChangeScript {
                script: syncsFlow.replace(selectiveSyncPage);
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepSyncFolder
            }
        }
    ]

    Component {
        id: syncPage

        SyncTypePage {}
    }

    Component {
        id: fullSyncPage

        FullSyncPage {}
    }

    Component {
        id: selectiveSyncPage

        SelectiveSyncPage {}
    }

}
