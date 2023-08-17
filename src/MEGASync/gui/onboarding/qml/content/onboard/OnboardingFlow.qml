// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// Local
import Onboard.Syncs_types 1.0

// C++
import Onboarding 1.0

StackView {
    id: onboardingFlow

    readonly property string register: "register"
    readonly property string syncs: "syncs"

    property bool isNewUser: false

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
        target: LogoutControllerAccess

        onLogout: {
            onboarding.state = onboarding.register;
        }
    }
}
