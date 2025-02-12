pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string finalStepSync: qsTranslate("OnboardingStrings", "Your sync has been set up and will automatically sync selected data whenever the MEGA Desktop App is running.")
    readonly property string finalStepSyncTitle: qsTranslate("OnboardingStrings", "Your sync has been set up")
    readonly property string invalidLocalPath: qsTranslate("OnboardingStrings", "Select a local folder to sync.")
    readonly property string invalidRemotePath: qsTranslate("OnboardingStrings", "Select a MEGA folder to sync.")
    readonly property string selectLocalFolder: qsTranslate("OnboardingStrings", "Select a local folder")
    readonly property string selectMEGAFolder: qsTranslate("OnboardingStrings", "Select a MEGA folder")
    readonly property string selectiveSyncDescription: qsTr("Sync folders in your local device with MEGA.")
    readonly property string selectiveSyncTitle: qsTr("Select folders to sync")
    readonly property string selectFolders: qsTr("Sync: Select folders")
    readonly property string sync: qsTranslate("OnboardingStrings", "Sync")
    readonly property string syncsWindowTitle: qsTr("Add sync")
    readonly property string confirm: qsTranslate("OnboardingString", "Confirm");
    readonly property string syncSetUp: qsTranslate("OnboardingString", "Sync set up");

}
