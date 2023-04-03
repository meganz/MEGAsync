import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

import Onboarding 1.0
import Onboard.Syncs_types 1.0

OnboardingFlowForm {
    id: onboardingFlow

    state: welcome

    welcomePage.continueButton.onClicked: {
        onboardingFlow.state = register;
    }

    Connections{
        target: Onboarding

        onLoginFinished: {
            onboardingFlow.state = syncs;
        }
    }
}
