// System
import QtQuick 2.12
import QtQuick.Controls 2.12

// C++
import Onboarding 1.0

TwoFAPageForm {

    signUpButton.onClicked: {
        registerFlow.state = register;
    }

    loginButton.onClicked: {
        state = code2FAStatus;
        loginButton.busyIndicatorVisible = true;
        Onboarding.onTwoFARequested(twoFAField.key);
    }

    loginButton.onAnimationFinished: {
        if(completed) {
            console.log("ANIMATION FINISHED 2FA"+completed)
            loginButton.busyIndicatorVisible = false;
            onboardingFlow.state = syncs;
        }
    }

    Connections {
        target: Onboarding

        onTwoFAFailed: {
            twoFAField.hasError = true;
            loginButton.busyIndicatorVisible = false;
            state = normalStatus;
        }

        onFetchingNodesProgress: {
            loginButton.progressValue = progress;
        }

        onLoginFinished: {
            state = fetchNodesStatus;
        }

    }
}
