pragma Singleton
import QtQuick 2.12

QtObject {

    // Paths

    readonly property url imagesPath: Qt.resolvedUrl("../../images/qml/")
    readonly property url imagesOnboardingPath: Qt.resolvedUrl(imagesPath + "/onboarding/")
    readonly property url imagesPasswordPath: Qt.resolvedUrl(imagesPath + "/password/")

    // Sorted alphabetically by dir

    readonly property url alertTriangle: imagesPath + "alert_triangle.svg"
    readonly property url arrowRight: imagesPath + "arrow_right.svg"
    readonly property url helpCircle: imagesPath + "help_circle.svg"
    readonly property url infoCircle: imagesPath + "info_circle.svg"
    readonly property url lock: imagesPath + "lock.svg"

    readonly property url building: imagesOnboardingPath + "building.svg"
    readonly property url checkCircleOutline: imagesOnboardingPath + "check_circle_outline.svg"
    readonly property url checkCircleSolid: imagesOnboardingPath + "check_circle_solid.svg"
    readonly property url cloud: imagesOnboardingPath + "cloud.svg"
    readonly property url database: imagesOnboardingPath + "database.svg"
    readonly property url edit: imagesOnboardingPath + "edit.svg"
    readonly property url folder: imagesOnboardingPath + "folder.svg"
    readonly property url fullSync: imagesOnboardingPath + "full_sync.svg"
    readonly property url fuse: imagesOnboardingPath + "fuse.svg"
    readonly property url infinity: imagesOnboardingPath + "infinity.svg"
    readonly property url installationTypeBackups: imagesOnboardingPath + "installation_type_backups.svg"
    readonly property url login: imagesOnboardingPath + "login.png"
    readonly property url mega: imagesOnboardingPath + "mega.svg"
    readonly property url pc: imagesOnboardingPath + "pc.svg"
    readonly property url pcMega: imagesOnboardingPath + "pc_mega.svg"
    readonly property url plusCircle: imagesOnboardingPath + "plus_circle.svg"
    readonly property url resume: imagesOnboardingPath + "resume.svg"
    readonly property url mail: imagesOnboardingPath + "mail.svg"
    readonly property url selectiveSync: imagesOnboardingPath + "selective_sync.svg"
    readonly property url shield_account_free: imagesOnboardingPath + "shield_account_free.svg"
    readonly property url shield_account_lite: imagesOnboardingPath + "shield_account_lite.svg"
    readonly property url shield_account_proI: imagesOnboardingPath + "shield_account_proI.svg"
    readonly property url shield_account_proII: imagesOnboardingPath + "shield_account_proII.svg"
    readonly property url shield_account_proIII: imagesOnboardingPath + "shield_account_proIII.svg"
    readonly property url sync: imagesOnboardingPath + "sync.svg"
    readonly property url twofa: imagesOnboardingPath + "twofa.png"
    readonly property url passwordVeryWeak: imagesPasswordPath + "very_weak.svg"
    readonly property url passwordWeak: imagesPasswordPath + "weak.svg"
    readonly property url passwordAverage: imagesPasswordPath + "average.svg"
    readonly property url passwordGood: imagesPasswordPath + "good.svg"
    readonly property url passwordStrong: imagesPasswordPath + "strong.svg"

}
