pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string cancel: qsTranslate("OnboardingStrings", "Cancel")
    readonly property string choose: qsTr("Choose")
    readonly property string done: qsTranslate("OnboardingStrings", "Done")
    readonly property string next: qsTranslate("OnboardingStrings", "Next")
    readonly property string skip: qsTranslate("OnboardingStrings", "Skip")
    readonly property string previous: qsTranslate("OnboardingStrings", "Previous")
    readonly property string tryAgain: qsTranslate("OnboardingStrings", "Try again")
    readonly property string viewInSettings: qsTranslate("OnboardingStrings", "View in Settings")
    readonly property string setExclusions: qsTranslate("OnboardingStrings", "Set Exclusions")
}
