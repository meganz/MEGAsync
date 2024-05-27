pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string backupsWindowTitle: qsTr("Add backup")
    readonly property string confirmFolders: qsTr("Confirm folders")
    readonly property string selectFolders: qsTr("Select folders")

    readonly property string addFolder: qsTranslate("OnboardingStrings", "Add folder")
    readonly property string backUp: qsTranslate("OnboardingStrings", "Back up")
    readonly property string backupFolders: qsTranslate("OnboardingStrings", "Backup Folders")
    readonly property string backupTo: qsTranslate("OnboardingStrings", "Backup to:")
    readonly property string changeFolder: qsTranslate("OnboardingStrings", "Change folder")
    readonly property string confirmBackupErrorDuplicated: qsTranslate("OnboardingStrings", "There is already a folder with the same name in this backup")
    readonly property string confirmBackupErrorRemote: qsTranslate("OnboardingStrings", "A folder with the same name already exists on your backups")
    readonly property string rename: qsTranslate("OnboardingStrings", "Rename")
    readonly property string selectAll: qsTranslate("OnboardingStrings", "[B]Select all[/B]")
    readonly property string selectBackupFoldersDescription: qsTranslate("OnboardingStrings", "Selected folders will automatically back up to the cloud when the desktop app is running.")
    readonly property string finalStepBackup: qsTranslate("OnboardingStrings", "Your backup has been set up and selected data will automatically backup whenever the desktop app is running.")
    readonly property string finalStepBackup2: qsTranslate("OnboardingStrings", "You can view your backups and their statuses under the Backup tab in Settings.")
    readonly property string finalStepBackupTitle: qsTranslate("OnboardingStrings", "Your backup is set up")

}
