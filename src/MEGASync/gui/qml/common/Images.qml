pragma Singleton
import QtQuick 2.15

QtObject {

    readonly property string none: ""

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //   Paths
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    readonly property url imagesPath: Qt.resolvedUrl("../../images/")
    readonly property url imagesTokenizablePath: Qt.resolvedUrl(imagesPath + "themed/tokenizable/")
    readonly property url imagesQmlPath: Qt.resolvedUrl(imagesPath + "qml/")
    readonly property url imagesOnboardingPath: Qt.resolvedUrl(imagesQmlPath + "onboarding/")
    readonly property url imagesSyncsPath: Qt.resolvedUrl(imagesQmlPath + "syncs/")
    readonly property url imagesGuestPath: Qt.resolvedUrl(imagesQmlPath + "guest/")
    readonly property url imagesExclusionsPath: Qt.resolvedUrl(imagesQmlPath + "sync_exclusions/")

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //   Image paths
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    readonly property url alertCircle: imagesQmlPath + "alert_circle.svg"
    readonly property url arrowUpDown: imagesQmlPath + "arrows-up-down-circle.svg"
    readonly property url alertCircleFilled: imagesQmlPath + "alert_circle_filled.svg"
    readonly property url alertTriangle: imagesQmlPath + "alert_triangle.svg"
    readonly property url alertTrianglePng: imagesQmlPath + "alert-triangle.png"
    readonly property url alertTrianglePng2: imagesQmlPath + "alert-triangle@2x.png"
    readonly property url alertTrianglePng3: imagesQmlPath + "alert-triangle@3x.png"
    readonly property url arrowRight: imagesQmlPath + "arrow_right.svg"
    readonly property url check: imagesQmlPath + "check.svg"
    readonly property url helpCircle: imagesQmlPath + "help_circle.svg"
    readonly property url indeterminate: imagesQmlPath + "indeterminate.svg"
    readonly property url lock: imagesQmlPath + "lock.svg"
    readonly property url crystalLock: imagesPath + "lock.png"
    readonly property url loader: imagesQmlPath + "loader.svg"
    readonly property url smallCircle: imagesQmlPath + "small_circle.svg"
    readonly property url tip: imagesQmlPath + "tip.svg"
    readonly property url megaOutline: imagesQmlPath + "mega_outline.svg"
    readonly property url mega: imagesQmlPath + "mega.svg"
    readonly property url trash: imagesQmlPath + "trash.svg"
    readonly property url xCircle: imagesQmlPath + "x_circle.svg"
    readonly property url smallCheck: imagesQmlPath + "small_check.svg"
    readonly property url checkCircle: imagesQmlPath + "check_circle.svg"
    readonly property url checkCircleFilled: imagesQmlPath + "check_circle_filled.svg"
    readonly property url warning: imagesQmlPath + "warning.png"
    readonly property url multidevices: imagesQmlPath + "multidevices.png"
    readonly property url twofa: imagesQmlPath + "lock.png"
    readonly property url eye: imagesQmlPath + "eye.svg"
    readonly property url eyeOff: imagesQmlPath + "eye-off.svg"
    readonly property url megaDevices: imagesQmlPath + "mega_3d_devices.png"
    readonly property url backupDevices: imagesQmlPath + "backup_3d_devices.png"
    readonly property url threeDots: imagesQmlPath + "three_dots.svg"
    readonly property url monitor: imagesTokenizablePath + "monitor_small_thin_outline.svg"
    readonly property url folderOpen: imagesQmlPath + "folder-open.svg"
    readonly property url pauseCircle: imagesQmlPath + "pause-circle.svg"
    readonly property url searchFilled: imagesQmlPath + "search.svg"
    readonly property url searchHollow: imagesQmlPath + "search-large.svg"
    readonly property url minusCircle: imagesQmlPath + "minus-circle.svg"
    readonly property url slashCircle: imagesQmlPath + "slash-circle.svg"
    readonly property url playCircle: imagesQmlPath + "play-circle.svg"
    readonly property url power: imagesQmlPath + "power.svg"
    readonly property url upArrow: imagesQmlPath + "up-arrow.svg"
    readonly property url downArrow: imagesQmlPath + "down-arrow.svg"
    readonly property url externalLink: imagesQmlPath + "external-link.svg"
    readonly property url offerBanner: imagesQmlPath + "offer_banner.png"

    readonly property url dialogMessageQuestion: imagesQmlPath + "dialog_message_question.svg"
    readonly property url dialogMessageInformation: imagesQmlPath + "dialog_message_information.svg"
    readonly property url dialogMessageWarning: imagesQmlPath + "dialog_message_warning.svg"
    readonly property url dialogMessageCritical: imagesQmlPath + "dialog_message_critical.svg"
    readonly property url dialogMessageSuccess: imagesQmlPath + "dialog_message_success.svg"
    readonly property url dialogMessageCross: imagesQmlPath + "dialog_message_cross.svg"

    readonly property url database: imagesOnboardingPath + "database.svg"
    readonly property url edit: imagesOnboardingPath + "edit.svg"
    readonly property url folder: imagesOnboardingPath + "folder.svg"
    readonly property url importFromCloud: imagesOnboardingPath + "cloud.svg"
    readonly property url installationTypeBackups: imagesOnboardingPath + "backup.png"
    readonly property url key: imagesOnboardingPath + "key.svg"
    readonly property url login: imagesOnboardingPath + "login.png"
    readonly property url pc: imagesOnboardingPath + "pc.svg"
    readonly property url pcMega: imagesOnboardingPath + "multidevices.png"
    readonly property url person: imagesOnboardingPath + "person.svg"
    readonly property url resume: imagesOnboardingPath + "resume.svg"
    readonly property url mail: imagesOnboardingPath + "mail.svg"
    readonly property url sync: imagesOnboardingPath + "sync.png"
    readonly property url syncIcon: imagesOnboardingPath + "syncb.svg"

    readonly property url exit: imagesGuestPath + "exit.svg"
    readonly property url guest: imagesGuestPath + "guest.png"
    readonly property url menu: imagesGuestPath + "menu.svg"
    readonly property url settings: imagesGuestPath + "settings.svg"
    readonly property url warningGuest: imagesGuestPath + "warning.svg"
    readonly property url settingUp: imagesGuestPath + "setting_up.png"
    readonly property url alertTriangleError: imagesGuestPath + "alert-triangle.png"

    readonly property url refresh: imagesExclusionsPath + "refresh.svg"
    readonly property url xSquare: imagesExclusionsPath + "x-square.svg"
    readonly property url editRule: imagesExclusionsPath + "edit.svg"
    readonly property url chevronDown: imagesExclusionsPath + "chevron-down.svg"
    readonly property url info: imagesExclusionsPath + "info.svg"




    readonly property url plus: imagesPath + "icon_plus.svg"
    readonly property url pause_circle_medium_thin_outline: "qrc:/pause-circle_medium_thin_outline.svg"
    readonly property url play_circle_medium_thin_outline: "qrc:/play-circle_medium_thin_outline.svg"
    readonly property url folder_search_small_thin_outline: "qrc:/folder-search_small_thin_outline.svg"
    readonly property url sync_01_small_thin_outline: "qrc:/sync-01_small_thin_outline.svg"
    readonly property url check_medium_regular_solid: "qrc:/check_medium_regular_solid.svg"
    readonly property url file_ignore_small_thin_outline: "qrc:/file-ignore_small_thin_outline.svg"
    readonly property url folder_small_thin_outline: "qrc:/folder_small_thin_outline.svg"
    readonly property url mega_medium_thin_outline: "qrc:/mega_medium_thin_outline.svg"
    readonly property url pause_thin_small_thin_outline: "qrc:/pause-thin_small_thin_outline.svg"
    readonly property url rotate_cw_small_thin_outline: "qrc:/rotate-cw_small_thin_outline.svg"
    readonly property url search_large_small_thin_outline: "qrc:/search-large_small_thin_outline.svg"
    readonly property url trash_small_thin_outline: "qrc:/trash_small_thin_outline.svg"
    readonly property url play_small_thin_outline: "qrc:/play_small_thin_outline.svg"
    readonly property url lightbulb_small_thin_outline: "qrc:/lightbulb_small_thin_outline.svg"
    readonly property url alert_circle_small_thin_outline: "qrc:/alert-circle_small_thin_outline.svg"
    readonly property url pen_2_small_thin_outline: "qrc:/pen-2_small_thin_outline.svg"
    readonly property url trash_off_small_thin_outline: "qrc:/trash-off_small_thin_outline.svg"
    readonly property url power_small_thin_outline: "qrc:/power_small_thin_outline.svg"
    readonly property url mega_small_thin_outline: "qrc:/MEGA_small_thin_outline.svg"
    readonly property url pause_small_thin_outline: "qrc:/pause_small_thin_outline.svg"
    readonly property url slash_circle_small_thin_outline: "qrc:/slash-circle_small_thin_outline.svg"
    readonly property url sync_plus_small_thin_outline: "qrc:/sync-plus_small_thin_outline.svg"
    readonly property url database_plus_small_thin_outline: "qrc:/database-plus_small_thin_outline.svg"
    readonly property url arrow_up_medium_regular_outline: "qrc:/arrow-up_medium_regular_outline.svg"
    readonly property url arrow_down_medium_regular_outline: "qrc:/arrow-down_medium_regular_outline.svg"
    readonly property url database_small_thin_outline: "qrc:/database_small_thin_outline.svg"
    readonly property url database_x_medium_thin_outline: "qrc:/database-x_medium_thin_outline.svg"

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //   Standard Icons
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    readonly property url standard_DirIcon: "image://standardicons/SP_DirIcon"

}
