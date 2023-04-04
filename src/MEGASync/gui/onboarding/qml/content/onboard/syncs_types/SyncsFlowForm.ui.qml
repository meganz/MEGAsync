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

    property alias stepPanel: stepPanel
    property alias rightPanel: rightPanel
    property alias mainPanel: mainPanel
    property alias syncsPanel: syncsPanel
    property alias finalPage: finalPage

    property alias installationTypePage: installationTypePage
    property alias computerNamePage: computerNamePage
    property alias selectBackupFoldersPage: selectBackupFoldersPage
    property alias confirmBackupFoldersPage: confirmBackupFoldersPage
    property alias syncPage: syncPage
    property alias selectiveSyncPage: selectiveSyncPage
    property alias fullSyncPage: fullSyncPage

    readonly property string computerName: "computerName"
    readonly property string syncType: "syncType"
    readonly property string syncs: "syncs"
    readonly property string confirmBackup: "confirmBackup"
    readonly property string selectBackup: "selectBackup"
    readonly property string selectiveSync: "selectiveSync"
    readonly property string fullSync: "fullSync"
    readonly property string finalState: "finalState"

    StackView {
        id: mainPanel

        anchors.fill: parent
        initialItem: syncsPanel

        ResumePage {
            id: finalPage

            visible: false
        }

        Rectangle {
            id: syncsPanel

            width: mainPanel.width
            height: mainPanel.height

            StepPanel {
                id: stepPanel

                z: 2
                width: 224
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                }
            }

            StackView {
                id: rightPanel

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

}
