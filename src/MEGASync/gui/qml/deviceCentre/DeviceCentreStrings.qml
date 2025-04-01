pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string windowTitle: /*qsTr*/("Device Centre")
    readonly property string addBackupLabel: /*qsTr*/("Add Backup")
    readonly property string addSyncLabel: /*qsTr*/("Add Sync")
    readonly property string sync: /*qsTr*/("Sync")
    readonly property string backup: /*qsTr*/("Backup")
    readonly property string name: /*qsTr*/("Name")
    readonly property string type: /*qsTr*/("Type")
    readonly property string size: /*qsTr*/("Size")
    readonly property string lastUpdated: /*qsTr*/("Last updated")
    readonly property string statusLabel: /*qsTr*/("Status")
    readonly property string contains: /*qsTr*/("Contains")
    readonly property string totalSize: /*qsTr*/("Total size")
    readonly property string statusUpToDate: /*qsTr*/("Up to date")
    readonly property string statusUpdating: /*qsTr*/("Updating...")
    readonly property string statusPaused: /*qsTr*/("Paused")
    readonly property string statusStopped: /*qsTr*/("Stopped")
    readonly property string actionOpenInMega: /*qsTr*/("Open in MEGA")
    readonly property string actionShowFinder: /*qsTr*/("Show in Finder")
    readonly property string actionShowExplorer: /*qsTr*/("Show in Explorer")
    readonly property string actionShowFolder: /*qsTr*/("Show in folder")
    readonly property string actionResume: /*qsTr*/("Resume")
    readonly property string actionPause: /*qsTr*/("Pause")
    readonly property string actionManageExclusions: /*qsTr*/("Manage exclusions")
    readonly property string actionQuickRescan: /*qsTr*/("Quick rescan")
    readonly property string actionDeepRescan: /*qsTr*/("Deep rescan")
    readonly property string actionRebootSync: /*qsTr*/("Reboot sync")
    readonly property string actionRebootBackup: /*qsTr*/("Reboot backup")
    readonly property string actionStopBackup: /*qsTr*/("Stop backup")
    readonly property string actionRemoveSync: /*qsTr*/("Remove synced folder")
    readonly property string renameDevice: /*qsTr*/("Rename Device")
    readonly property string rename: /*qsTr*/("Rename")
    readonly property string troubleshoot: /*qsTr*/("Troubleshoot")
    readonly property string view: /*qsTr*/("View")
    readonly property string resolutionModeTitle: /*qsTr*/("Issue resolution mode")
    readonly property string smart: /*qsTr*/("Smart")
    readonly property string advanced: /*qsTr*/("Advanced")
    readonly property string exclusionRulesTitle: /*qsTr*/("Exclusion rules")
    readonly property string smartDescription: /*qsTr*/("Let MEGA resolve sync issues automatically")
    readonly property string advancedDescription: /*qsTr*/("Get full control of backup issues")
    readonly property string exclusionRulesDescription: /*qsTr*/("The exclusion rules you set up in a previous version of the app will be applied to all of your syncs and backups. Any rules created since then will be overwritten.")
    readonly property string learnMore: /*qsTr*/("Learn more")
    readonly property string applyPreviousRules: /*qsTr*/("Apply previous rules")
    readonly property string localPathLabel: /*qsTr*/("Local path: %1")
    readonly property string remotePathLabel: /*qsTr*/("Remote path: %1")
    readonly property string noConnectionTitle: /*qsTr*/("You have no connections")
    readonly property string noConnectionDescription: /*qsTr*/("Create your first sync or backup to start managing your data across all your devices")
    readonly property string rebootDialogTitleSync: /*qsTr*/("Reboot sync?")
    readonly property string rebootDialogTitleBackup: /*qsTr*/("Reboot backup?")
    readonly property string rebootDialogBodySync: /*qsTr*/("This action automatically restarts your sync, which can resolve sync issues. Files that are the same on both sides of the sync will not be affected.
[BR][BR]It may take some time to complete synchronising. If this doesn't resolve your sync issues, report the issue to Support.")
    readonly property string rebootDialogBodyBackup: /*qsTr*/("This action automatically restarts your backup, which can resolve backup issues. Files that are the same on both sides of the backup will not be affected.
[BR][BR]It may take some time to complete synchronising. If this doesn't resolve your backup issues, report the issue to Support.")

    function folderCount(count) {
        return /*qsTr*/("%n folders"/*, "", count*/)
    }
}
