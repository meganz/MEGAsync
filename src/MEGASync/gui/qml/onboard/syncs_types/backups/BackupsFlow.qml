// System
import QtQuick 2.15
import QtQuick.Controls 2.15

// Local
import onboard 1.0
import Onboard.Syncs_types.Left_panel 1.0

// C++
import BackupsProxyModel 1.0

Item {
    id: root

    signal backupFlowMoveToFinal
    signal backupFlowMoveToBack

    readonly property string selectBackup: "selectBackup"
    readonly property string confirmBackup: "confirmBackup"

    state: selectBackup
    states: [
        State {
            name: selectBackup
            StateChangeScript {
                script: view.replace(selectBackupFoldersPage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step3;
                step3Text: OnboardingStrings.backupSelectFolders;
                step4Text: OnboardingStrings.confirm;
            }
        },
        State {
            name: confirmBackup
            StateChangeScript {
                script: view.replace(confirmBackupFoldersPage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step4;
                step3Text: OnboardingStrings.backupSelectFolders;
                step4Text: OnboardingStrings.backupConfirm;
            }
        }
    ]

    StackViewBase {
        id: view

        anchors.fill: parent

        Component {
            id: selectBackupFoldersPage

            SelectFoldersPage {}
        }

        Component {
            id: confirmBackupFoldersPage

            ConfirmFoldersPage {}
        }
    }

    BackupsProxyModel {
        id: backupsProxyModel
    }

    /*
    * Navigation connections
    */
    Connections {
        id: confirmFolderBackupNavigationConnection
        target: view.currentItem
        ignoreUnknownSignals: true

        function onConfirmFoldersMoveToSelect() {
            root.state = root.selectBackup
            backupsProxyModel.selectedFilterEnabled = false;
        }

        function onConfirmFoldersMoveToSuccess() {
            root.backupFlowMoveToFinal()
        }
    }

    Connections {
        id: selectFolderBackupNavigationConnection
        target: view.currentItem
        ignoreUnknownSignals: true

        function onSelectFolderMoveToBack() {
            if(syncsPanel.navInfo.comesFromResumePage) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                root.backupFlowMoveToFinal()
            } else {
                root.backupFlowMoveToBack()
            }
        }

        function onSelectFolderMoveToConfirm() {
            root.state = root.confirmBackup
        }
    }
}
