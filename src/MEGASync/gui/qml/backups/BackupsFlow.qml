import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import onboard 1.0

import BackupsProxyModel 1.0

Item {
    id: root

    readonly property string selectBackup: "selectBackup"
    readonly property string confirmBackup: "confirmBackup"

    property bool isOnboarding: false

    // Added to avoid qml warning.
    function setInitialFocusPosition() {}

    signal backupFlowMoveToFinal(bool success)
    signal backupFlowMoveToBack

    state: selectBackup
    states: [
        State {
            name: selectBackup
            StateChangeScript {
                script: view.replace(selectBackupFoldersPage);
            }
        },
        State {
            name: confirmBackup
            StateChangeScript {
                script: view.replace(confirmBackupFoldersPage);
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
            id: selectBackupFoldersPage

            SelectFoldersPage {
                footerButtons.leftSecondary.text: Strings.cancel
                footerButtons.rightSecondary.visible: isOnboarding
            }
        }

        Component {
            id: confirmBackupFoldersPage

            ConfirmFoldersPage {
                footerButtons.leftSecondary.visible: isOnboarding
                backupsProxyModelRef: backupsProxyModel
            }
        }
    }

    BackupsProxyModel {
        id: backupsProxyModel
    }

    Connections {
        id: selectFolderBackupNavigationConnection

        target: view.currentItem
        ignoreUnknownSignals: true

        function onSelectFolderMoveToBack() {
            root.backupFlowMoveToBack();
        }

        function onSelectFolderMoveToConfirm() {
            backupsProxyModel.selectedFilterEnabled = true;
            root.state = root.confirmBackup;
        }
    }

    Connections {
        id: confirmFolderBackupNavigationConnection

        target: view.currentItem
        ignoreUnknownSignals: true

        function onConfirmFoldersMoveToSelect() {
            backupsProxyModel.selectedFilterEnabled = false;
            root.state = root.selectBackup;
        }

        function onConfirmFoldersMoveToFinal(success) {
            root.backupFlowMoveToFinal(success);
        }
    }
}
