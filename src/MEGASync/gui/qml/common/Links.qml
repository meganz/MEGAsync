pragma Singleton
import QtQuick 2.15

QtObject {
    readonly property url megaNZ: "https://mega.nz/"
    readonly property url recovery: megaNZ + "recovery"
    readonly property url backupCentre: megaNZ + "fm/devices"

    readonly property url megaIO: "https://mega.io/"
    readonly property url terms: megaIO + "terms"
    readonly property url contact: megaIO + "contact"
    readonly property url security: megaIO + "security"

    readonly property url helpMegaIO: "https://help.mega.io/"
    readonly property url installAppsDesktop: helpMegaIO + "installs-apps/desktop/"
    readonly property url createBackup: installAppsDesktop + "create-backup"
    readonly property url setUpSyncs: installAppsDesktop + "set-up-syncs"
    readonly property url helpSyncs: installAppsDesktop + "set-up-syncs"
    readonly property url helpBackups: installAppsDesktop + "create-backup"
}
