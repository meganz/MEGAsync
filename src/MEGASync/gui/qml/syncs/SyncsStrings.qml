pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string canNotSyncPermissionError: qsTr("Folder can’t be synced as you don’t have permissions to create a new folder. To continue, select an existing folder.")
    readonly property string finalStepSync: qsTr("Your sync has been set up and will automatically sync selected data whenever the MEGA Desktop App is running.")
    readonly property string finalStepSyncTitle: qsTr("Your sync has been set up")
    readonly property string fullSync: qsTr("Full sync")
    readonly property string fullSyncButtonDescription: qsTr("Sync your entire MEGA account with your local device.")
    readonly property string fullSyncDescription: qsTr("Sync your entire MEGA Cloud drive with a local device.")
    readonly property string invalidLocalPath: qsTr("Select a local folder to sync.")
    readonly property string invalidRemotePath: qsTr("Select a MEGA folder to sync.")
    readonly property string selectLocalFolder: qsTr("Select a local folder")
    readonly property string selectMEGAFolder: qsTr("Select a MEGA folder")
    readonly property string selectiveSyncButtonDescription: qsTr("Sync selected folders in your MEGA account with your local device.")
    readonly property string selectiveSyncDescription: qsTr("Sync specific folders in your MEGA Cloud drive with a local device.")
    readonly property string selectiveSync: qsTr("Selective sync")
    readonly property string sync: qsTr("Sync")
    readonly property string syncTitle: qsTr("Choose sync type")
    readonly property string syncType: qsTr("Choose type")
    readonly property string syncsWindowTitle: qsTr("Add sync")

}
