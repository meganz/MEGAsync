pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string windowTitle: qsTr("Device Centre")
    readonly property string addBackupLabel: qsTr("Add Backup")
    readonly property string addSyncLabel: qsTr("Add Sync")
    readonly property string sync: qsTr("Sync")
    readonly property string backup: qsTr("Backup")
    readonly property string name: qsTr("Name")
    readonly property string type: qsTr("Type")
    readonly property string size: qsTr("Size")
    readonly property string lastUpdated: qsTr("Last updated")
    readonly property string statusLabel: qsTr("Status")
    readonly property string contains: qsTr("Contains")
    readonly property string totalSize: qsTr("Total size")
    readonly property string statusUpToDate: qsTr("Up to date")
    readonly property string statusUpdating: qsTr("Updating...")
    readonly property string statusPaused: qsTr("Paused")
    readonly property string statusStopped: qsTr("Stopped")
    readonly property string actionOpenInMega: qsTr("Open in MEGA")
    readonly property string actionShowFinder: qsTr("Show in Finder")
    readonly property string actionShowExplorer: qsTr("Show in Explorer")
    readonly property string actionShowFolder: qsTr("Show in folder")
    readonly property string actionResume: qsTr("Resume")
    readonly property string actionPause: qsTr("Pause")
    readonly property string actionManageExclusions: qsTr("Manage exclusions")
    readonly property string actionQuickRescan: qsTr("Quick rescan")
    readonly property string actionDeepRescan: qsTr("Deep rescan")
    readonly property string actionStopBackup: qsTr("Stop backup")
    readonly property string actionRemoveSync: qsTr("Remove synced folder")
    readonly property string renameDevice: qsTr("Rename Device")
    readonly property string rename: qsTr("Rename")
    readonly property string view: qsTr("View")

    function folderCount(count) {
        return qsTr("%n folders", "", count)
    }
}
