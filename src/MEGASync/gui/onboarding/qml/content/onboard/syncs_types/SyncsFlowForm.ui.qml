import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Onboard.Syncs_types.Left_panel 1.0
import Onboard.Syncs_types.Syncs 1.0
import Onboard.Syncs_types.Backups 1.0

import Components 1.0 as Custom
import Common 1.0

Item {
    id: syncsStack

    property alias installationTypePage: installationTypePage
    property alias computerNamePage: computerNamePage
    property alias selectBackupFoldersPage: selectBackupFoldersPage
    property alias confirmBackupFoldersPage: confirmBackupFoldersPage
    property alias syncPage: syncPage
    property alias selectiveSyncPage: selectiveSyncPage
    property alias fullSyncPage: fullSyncPage
    property alias finalPage: finalPage

    readonly property string computerName: "computerName"
    readonly property string syncType: "syncType"
    readonly property string syncs: "syncs"
    readonly property string confirmBackup: "confirmBackup"
    readonly property string selectBackup: "selectBackup"
    readonly property string selectiveSync: "selectiveSync"
    readonly property string fullSync: "fullSync"
    readonly property string finalState: "finalState"


    /*
     * Object properties
     */
    anchors.fill: parent


    /*
         * Child objects
         */
    states: [
        State {
            name: computerName
            PropertyChanges {
                target: computerNamePage
                visible: true
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.step1ComputerName
            }
        },
        State {
            name: syncType
            PropertyChanges {
                target: installationTypePage
                visible: true
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.step2InstallationType
            }
            PropertyChanges {
                target: syncsPanel
                visible: true
            }
        },
        State {
            name: syncs
            PropertyChanges {
                target: syncPage
                visible: true
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepSelectSyncType
            }
        },
        State {
            name: selectiveSync
            PropertyChanges {
                target: selectiveSyncPage
                visible: true
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepSyncFolder
            }
        },
        State {
            name: fullSync
            PropertyChanges {
                target: fullSyncPage
                visible: true
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepSyncFolder
            }
        },
        State {
            name: selectBackup
            PropertyChanges {
                target: selectBackupFoldersPage
                visible: true
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepBackupsSelectFolders
            }
        },
        State {
            name: confirmBackup
            PropertyChanges {
                target: confirmBackupFoldersPage
                visible: true
            }
            PropertyChanges {
                target: stepPanel
                state: stepPanel.stepBackupsConfirm
            }
        },
        State {
            name: finalState
            PropertyChanges {
                target: finalPage
                visible: true
            }
            PropertyChanges {
                target: syncsPanel
                visible: false
            }
        }
    ]

    Rectangle{
        id: contentItem
        anchors.fill: parent

        ResumePage {
            id: finalPage
            visible: false
        }
        Rectangle {
            id:syncsPanel
            anchors.fill:parent
        StepPanel {

            id: stepPanel
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            width: 224
        }

        Rectangle {
            id: rightPanel
            objectName: "CONTENT ITEM RECTANGLE"
            anchors {
                left: stepPanel.right
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
            ComputerNamePage {
                id: computerNamePage
                Layout.fillHeight: true
                Layout.fillWidth: true
                visible: true
            }

            InstallationTypePage {
                id: installationTypePage

                visible: false
            }

            SyncTypePage {
                id: syncPage

                visible: false
            }

            FullSyncPage {
                id: fullSyncPage

                visible: false
            }

            SelectiveSyncPage {
                id: selectiveSyncPage

                visible: false
            }

            SelectFoldersPage {
                id: selectBackupFoldersPage

                visible: false
            }

            ConfirmFoldersPage {
                id: confirmBackupFoldersPage

                visible: false
            }
        }
        }
    }
} // StackView

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

