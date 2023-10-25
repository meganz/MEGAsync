// System
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

// Local
import Onboard.Syncs_types 1.0

// C++
import Onboarding 1.0
import LoginController 1.0

StackView {
    id: onboardingFlow

    readonly property string register: "register"
    readonly property string syncs: "syncs"

    state: register

    states: [
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

    replaceEnter: Transition {
        PropertyAnimation {
            property: "opacity"
            from: 0
            to:1
            duration: 100
            easing.type: Easing.OutQuad
        }
    }
    replaceExit: Transition {
        PropertyAnimation {
            property: "opacity"
            from: 1
            to:0
            duration: 100
            easing.type: Easing.InQuad
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

        MainFlow {
            width: onboardingFlow.width
            height: onboardingFlow.height
        }
    }

    Connections{
        target: logoutControllerAccess

        function onLogout() {
            onboarding.state = onboarding.register;
        }
    }
}
