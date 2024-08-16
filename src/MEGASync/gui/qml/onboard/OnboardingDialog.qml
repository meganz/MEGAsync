import common 1.0

import OnboardingQmlDialog 1.0
import SyncInfo 1.0

OnboardingQmlDialog {
    id: window

    readonly property int syncOrigin: SyncInfo.ONBOARDING_ORIGIN

    title: OnboardingStrings.setUpMEGA
    visible: true
    modality: Qt.NonModal
    width: 800
    height: 560
    maximumHeight: 560
    maximumWidth: 800
    minimumHeight: 560
    minimumWidth: 800

    OnboardingFlow {
        id: onboarding

        anchors.fill: parent
    }
}
