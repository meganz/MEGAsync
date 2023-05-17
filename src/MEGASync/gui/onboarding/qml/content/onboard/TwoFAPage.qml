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
        loginButton.icons.busyIndicatorVisible = true;
        Onboarding.onTwoFARequested(twoFAField.key);
    }

    loginButton.progress.onAnimationFinished: {
        if(completed) {
            console.log("ANIMATION FINISHED 2FA"+completed)
            loginButton.icons.busyIndicatorVisible = false;
            onboardingFlow.state = syncs;
        }
    }

    Connections {
        target: Onboarding

        onTwoFAFailed: {
            twoFAField.hasError = true;
            loginButton.icons.busyIndicatorVisible = false;
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
