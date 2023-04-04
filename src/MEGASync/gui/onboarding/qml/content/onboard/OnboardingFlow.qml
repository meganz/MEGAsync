import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

import Onboarding 1.0
import Onboard.Syncs_types 1.0

StackView {
    id: onboardingFlow

    readonly property string welcome: "welcome"
    readonly property string register: "register"
    readonly property string syncs: "syncs"

    state: register

    states: [
        State {
            name: welcome
            StateChangeScript {
                script: replace(welcomeComponent);
            }
        },
        State {
            name: register
            StateChangeScript {
                script: replace(registerComponent);
            }
        },
        State {
            name: syncs
            StateChangeScript {
                script: replace(syncsComponent);
            }
        }
    ]

    Component {
        id: welcomeComponent

        Welcome {
            width: onboardingFlow.width
            height: onboardingFlow.height

            continueButton.onClicked: {
                onboardingFlow.state = register;
            }
        }
    }

    Component {
        id: registerComponent

        RegisterFlow {
            width: onboardingFlow.width
            height: onboardingFlow.height
        }
    }

    Component {
        id: syncsComponent

        SyncsFlow {
            width: onboardingFlow.width
            height: onboardingFlow.height
        }
    }

    Connections{
        target: Onboarding

        onLoginFinished: {
            onboardingFlow.state = syncs;
        }
    }
}
