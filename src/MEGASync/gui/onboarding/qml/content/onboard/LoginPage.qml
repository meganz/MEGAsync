// System
import QtQuick 2.12

// QML common
import Components 1.0 as Custom

// Local
import Onboarding 1.0

// C++
import Onboard 1.0

LoginPageForm {
    id: root

    property bool loginAttempt: false
    property bool twoFARequired: false

    Keys.onEnterPressed: {
        loginButton.forceActiveFocus();
        loginButton.clicked();
    }

    Keys.onReturnPressed: {
        loginButton.forceActiveFocus();
        loginButton.clicked();
    }

    loginButton.onClicked: {
        var error = false;

        var valid = email.valid();
        if(!valid) {
            error = true;
            email.hint.text = OnboardingStrings.errorValidEmail;
        }
        email.error = !valid;
        email.hint.visible = !valid;

        valid = (password.text.length !== 0);
        if(!valid) {
            error = true;
            password.hint.text = OnboardingStrings.errorEmptyPassword;
        }
        password.error = !valid;
        password.hint.visible = !valid;

        if(error) {
            return;
        }

        loginButton.icons.busyIndicatorVisible = true;
        state = logInStatus;
        Onboarding.onLoginClicked({ [Onboarding.RegisterForm.EMAIL]: email.text,
                                    [Onboarding.RegisterForm.PASSWORD]: password.text })
        loginAttempt = true;
    }

    loginButton.progress.onAnimationFinished: {
        if(completed) {
            loginButton.icons.busyIndicatorVisible = false;
            state = normalStatus;
            onboardingFlow.state = syncs;
        }
    }

    signUpButton.onClicked: {
        registerFlow.state = register;
    }

    password.onTextChanged: {
        if(loginAttempt && !email.hint.visible && email.error) {
            email.error = false;
        }
    }

    Connections {
        target: Onboarding

        onUserPassFailed: {
            root.enabled = true;
            email.error = true;
            password.error = true;
            password.hint.text = OnboardingStrings.errorLogin;
            password.hint.visible = true;
            loginButton.icons.busyIndicatorVisible = false;
            state = normalStatus;
        }

        onFetchingNodesProgress: {
            console.log("LOGIN PAGE progress:"+progress)
            loginButton.progress.value = progress;
        }

        onLoginFinished: {
            state = fetchNodesStatus;
        }

        onTwoFARequired: {
            loginButton.icons.busyIndicatorVisible = false;
            registerFlow.state = twoFA;
        }
    }
}
