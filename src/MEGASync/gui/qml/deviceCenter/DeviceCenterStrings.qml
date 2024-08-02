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
    readonly property string dateAdded: qsTr("Date added")
    readonly property string lastUpdated: qsTr("Last updated")
    readonly property string statusLabel: qsTr("Status")
    readonly property string contains: qsTr("Contains")
    readonly property string totalSize: qsTr("Total size")
    readonly property string statusUpToDate: qsTr("Up to date")
    readonly property string statusUpdating: qsTr("Updating...")
    readonly property string statusPaused: qsTr("Paused")
    readonly property string statusStopped: qsTr("Stopped")
    readonly property string deviceCentre: qsTr("Device Centre")

    function folderCount(count) {
        return qsTr("%n folders", "", count)
    }
}
