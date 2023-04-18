pragma Singleton
import QtQuick 2.12

QtObject {

    readonly property url imagesPath: Qt.resolvedUrl("../../images/qml/")
    readonly property url imagesOnboardingPath: Qt.resolvedUrl(imagesPath + "/onboarding/")

    readonly property url alertTriangle: imagesPath + "alert_triangle.svg"
    readonly property url arrowRight: imagesPath + "arrow_right.svg"
    readonly property url helpCircle: imagesPath + "help_circle.svg"

    readonly property url checkCircleOutline: imagesOnboardingPath + "check_circle_outline.svg"
    readonly property url checkCircleSolid: imagesOnboardingPath + "check_circle_solid.svg"
    readonly property url cloud: imagesOnboardingPath + "cloud.svg"
    readonly property url folder: imagesOnboardingPath + "folder.svg"
    readonly property url fullSync: imagesOnboardingPath + "full_sync.svg"
    readonly property url fuse: imagesOnboardingPath + "fuse.svg"
    readonly property url installationTypeBackups: imagesOnboardingPath + "installation_type_backups.svg"
    readonly property url lock: imagesOnboardingPath + "lock.svg"
    readonly property url login: imagesOnboardingPath + "login.png"
    readonly property url mega: imagesOnboardingPath + "mega.svg"
    readonly property url pc: imagesOnboardingPath + "pc.svg"
    readonly property url pcMega: imagesOnboardingPath + "pc_mega.svg"
    readonly property url plusCircle: imagesOnboardingPath + "plus_circle.svg"
    readonly property url resume: imagesOnboardingPath + "resume.svg"
    readonly property url selectiveSync: imagesOnboardingPath + "selective_sync.svg"
    readonly property url shield: imagesOnboardingPath + "shield.svg"
    readonly property url sync: imagesOnboardingPath + "sync.svg"
    readonly property url twofa: imagesOnboardingPath + "twofa.png"

}
