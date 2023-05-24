// System
import QtQuick 2.12
import QtQuick.Controls 2.12

// Local
import Onboard.Syncs_types.Left_panel 1.0

// C++
import BackupsProxyModel 1.0

StackView {
    id: backupsFlow

    property StepPanel stepLeftPanel

    readonly property string selectBackup: "selectBackup"
    readonly property string confirmBackup: "confirmBackup"

    onStepLeftPanelChanged: {
        state = selectBackup;
    }

    states: [
        State {
            name: selectBackup
            StateChangeScript {
                script: backupsFlow.replace(selectBackupFoldersPage);
            }
            PropertyChanges {
                target: stepLeftPanel
                state: stepLeftPanel.stepBackupsSelectFolders
            }
        },
        State {
            name: confirmBackup
            StateChangeScript {
                script: backupsFlow.replace(confirmBackupFoldersPage);
            }
            PropertyChanges {
                target: stepLeftPanel
                state: stepLeftPanel.stepBackupsConfirm
            }
        }
    ]

    BackupsProxyModel {
        id: backupsProxyModel
    }

    Component {
        id: selectBackupFoldersPage

        SelectFoldersPage {
            proxyModel: backupsProxyModel
        }
    }

    Component {
        id: confirmBackupFoldersPage

        ConfirmFoldersPage {
            proxyModel: backupsProxyModel
        }
    }

}
