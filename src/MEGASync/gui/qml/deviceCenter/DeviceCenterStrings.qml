pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string windowTitle: qsTr("Device Centre")
    readonly property string addBackupLabel: qsTr("Add Backup")
    readonly property string addSyncLabel: qsTr("Add Sync")
    readonly property string statusLabel: qsTr("Status")
    readonly property string contains: qsTr("Contains")
    readonly property string totalSize: qsTr("Total size")
    readonly property string statusUpToDate: qsTr("Up to date")
    readonly property string statusUpdating: qsTr("Updating...")
    readonly property string statusPaused: qsTr("Paused")
    readonly property string statusStopped: qsTr("Stopped")

    function folderCount(count) {
        return qsTr("%n folders", "", count)
    }
}
