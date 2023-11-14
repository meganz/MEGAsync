// Local
import Onboard 1.0

// C++
import OnboardingQmlDialog 1.0

OnboardingQmlDialog {
    id: onboardingWindow

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
