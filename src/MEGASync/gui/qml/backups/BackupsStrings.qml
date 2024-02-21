pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string addFolder: qsTr("Add folder")
    readonly property string backUp: qsTr("Back up")
    readonly property string backupFolders: qsTr("Backup Folders")
    readonly property string backupTo: qsTr("Backup to:")
    readonly property string changeFolder: qsTr("Change folder")
    readonly property string confirmFolders: qsTr("Confirm folders")
    readonly property string backupsWindowTitle: qsTr("Add backup")
    readonly property string confirmBackupErrorDuplicated: qsTr("There is already a folder with the same name in this backup")
    readonly property string confirmBackupErrorRemote: qsTr("A folder with the same name already exists on your backups")
    readonly property string confirmBackupFoldersTitle: qsTr("Confirm folders to back up")
    readonly property string rename: qsTr("Rename")
    readonly property string selectAll: qsTr("[B]Select all[/B]")
    readonly property string selectFolders: qsTr("Select folders")
    readonly property string selectBackupFoldersTitle: qsTr("Select folders to back up")
    readonly property string selectBackupFoldersDescription: qsTr("Selected folders will automatically back up to the cloud when the desktop app is running.")
    readonly property string finalStepBackup: qsTr("Your backup has been set up and selected data will automatically backup whenever the desktop app is running.")
    readonly property string finalStepBackup2: qsTr("You can view your backups and their statuses under the Backup tab in Settings.")
    readonly property string finalStepBackupTitle: qsTr("Your backup is set up")

}
