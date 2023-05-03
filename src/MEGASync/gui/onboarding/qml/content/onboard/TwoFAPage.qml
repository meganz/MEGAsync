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
        loginButton.animationDuration = 2000;
        loginButton.progressValue = 0.9;
        loginButton.busyIndicatorVisible = true;
        Onboarding.onTwoFARequested(twoFAField.key);
    }

    loginButton.onAnimationFinished: {
        if(completed)
        {
            loginButton.busyIndicatorVisible = false;
            loginButton.progressValue = 0;
            onboardingFlow.state = syncs;
        }
    }

    Connections {
        target: Onboarding

        onTwoFAFailed: {
            twoFAField.hasError = true;
        }

        onFetchingNodesProgress: {
            console.log("two fa progress:"+progress)
            loginButton.progressValue = progress;
        }

        onLoginFinished: {
            loginButton.animationDuration = 1;
            loginButton.progressValue = 0; //start fetching nodes
            state = fetchNodesStatus;
        }

    }
}
