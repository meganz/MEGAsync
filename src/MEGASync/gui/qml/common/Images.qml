pragma Singleton
import QtQuick 2.12

QtObject {

    readonly property string none: ""

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //   Paths
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    readonly property url imagesPath: Qt.resolvedUrl("../../images/qml/")
    readonly property url imagesOnboardingPath: Qt.resolvedUrl(imagesPath + "/onboarding/")
    readonly property url imagesGuestPath: Qt.resolvedUrl(imagesPath + "/guest/")

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //   Image paths
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    readonly property url alertCircle: imagesPath + "alert_circle.svg"
    readonly property url alertTriangle: imagesPath + "alert_triangle.svg"
    readonly property url arrowRight: imagesPath + "arrow_right.svg"
    readonly property url check: imagesPath + "check.svg"
    readonly property url helpCircle: imagesPath + "help_circle.svg"
    readonly property url indeterminate: imagesPath + "indeterminate.svg"
    readonly property url lock: imagesPath + "lock.svg"
    readonly property url loader: imagesPath + "loader.svg"
    readonly property url smallCircle: imagesPath + "small_circle.svg"
    readonly property url tip: imagesPath + "tip.svg"
    readonly property url megaOutline: imagesPath + "mega_outline.svg"
    readonly property url mega: imagesPath + "mega.svg"
    readonly property url trash: imagesPath + "trash.svg"
    readonly property url xCircle: imagesPath + "x_circle.svg"
    readonly property url smallCheck: imagesPath + "small_check.svg"
    readonly property url checkCircle: imagesPath + "check_circle.svg"
    readonly property url warning: imagesPath + "warning.png"
    readonly property url twofa: imagesPath + "lock.png"

    readonly property url building: imagesOnboardingPath + "building.svg"
    readonly property url database: imagesOnboardingPath + "database.svg"
    readonly property url edit: imagesOnboardingPath + "edit.svg"
    readonly property url folder: imagesOnboardingPath + "folder.svg"
    readonly property url fullSync: imagesOnboardingPath + "full_sync.svg"
    readonly property url infinity: imagesOnboardingPath + "infinity.svg"
    readonly property url installationTypeBackups: imagesOnboardingPath + "installation_type_backups.svg"
    readonly property url key: imagesOnboardingPath + "key.svg"
    readonly property url login: imagesOnboardingPath + "login.png"
    readonly property url pc: imagesOnboardingPath + "pc.svg"
    readonly property url pcMega: imagesOnboardingPath + "pc_mega.svg"
    readonly property url person: imagesOnboardingPath + "person.svg"
    readonly property url plus: imagesOnboardingPath + "plus.svg"
    readonly property url resume: imagesOnboardingPath + "resume.svg"
    readonly property url mail: imagesOnboardingPath + "mail.svg"
    readonly property url selectiveSync: imagesOnboardingPath + "selective_sync.svg"
    readonly property url shield_account_free: imagesOnboardingPath + "shield_account_free.svg"
    readonly property url shield_account_lite: imagesOnboardingPath + "shield_account_lite.svg"
    readonly property url shield_account_proI: imagesOnboardingPath + "shield_account_proI.svg"
    readonly property url shield_account_proII: imagesOnboardingPath + "shield_account_proII.svg"
    readonly property url shield_account_proIII: imagesOnboardingPath + "shield_account_proIII.svg"
    readonly property url sync: imagesOnboardingPath + "sync.svg"
    readonly property url syncIcon: imagesOnboardingPath + "syncb.svg"
    readonly property url okIcon: imagesOnboardingPath + "OKIcon.png"

    readonly property url exit: imagesGuestPath + "exit.svg"
    readonly property url guest: imagesGuestPath + "guest.png"
    readonly property url menu: imagesGuestPath + "menu.svg"
    readonly property url settings: imagesGuestPath + "settings.svg"
    readonly property url warningGuest: imagesGuestPath + "warning.png"
    readonly property url settingUp: imagesGuestPath + "setting_up.png"

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //   Standard Icons
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    readonly property url standard_DirIcon: "image://standardicons/SP_DirIcon"

}
