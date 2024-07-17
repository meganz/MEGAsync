pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string canNotSyncPermissionError: qsTranslate("OnboardingStrings", "Folder can’t be synced as you don’t have permissions to create a new folder. To continue, select an existing folder.")
    readonly property string finalStepSync: qsTranslate("OnboardingStrings", "Your sync has been set up and will automatically sync selected data whenever the MEGA Desktop App is running.")
    readonly property string finalStepSyncTitle: qsTranslate("OnboardingStrings", "Your sync has been set up")
    readonly property string fullSync: qsTranslate("OnboardingStrings", "Full sync")
    readonly property string fullSyncButtonDescription: qsTranslate("OnboardingStrings", "Sync your entire MEGA Cloud drive with your local device.")
    readonly property string fullSyncDescription: qsTranslate("OnboardingStrings", "Sync your entire MEGA Cloud drive with a local device.")
    readonly property string invalidLocalPath: qsTranslate("OnboardingStrings", "Select a local folder to sync.")
    readonly property string invalidRemotePath: qsTranslate("OnboardingStrings", "Select a MEGA folder to sync.")
    readonly property string selectLocalFolder: qsTranslate("OnboardingStrings", "Select a local folder")
    readonly property string selectMEGAFolder: qsTranslate("OnboardingStrings", "Select a MEGA folder")
    readonly property string selectiveSyncButtonDescription: qsTranslate("OnboardingStrings", "Sync selected folders in your MEGA Cloud drive with your local device.")
    readonly property string selectiveSyncDescription: qsTranslate("OnboardingStrings", "Sync specific folders in your MEGA Cloud drive with a local device.")
    readonly property string selectiveSync: qsTranslate("OnboardingStrings", "Selective sync")
    readonly property string sync: qsTranslate("OnboardingStrings", "Sync")
    readonly property string syncTitle: qsTranslate("OnboardingStrings", "Choose sync type")

    readonly property string syncsWindowTitle: qsTr("Add sync")

}
