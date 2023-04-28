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

    Keys.onEnterPressed: {
        loginButton.clicked();
    }

    Keys.onReturnPressed: {
        loginButton.clicked();
    }

    loginButton.onClicked: {
        var error = false;

        var valid = email.valid();
        if(!valid) {
            error = true;
            email.hintText = OnboardingStrings.errorValidEmail;
        }
        email.showType = !valid;
        email.hintVisible = !valid;

        valid = (password.text.length !== 0);
        if(!valid) {
            error = true;
            password.hintText = OnboardingStrings.errorEmptyPassword;
        }
        password.showType = !valid;
        password.hintVisible = !valid;

        if(error) {
            return;
        }

        root.enabled = false;
        Onboarding.onLoginClicked({ [Onboarding.RegisterForm.EMAIL]: email.text,
                                    [Onboarding.RegisterForm.PASSWORD]: password.text })
        loginAttempt = true;
    }

    signUpButton.onClicked: {
        registerFlow.state = register;
    }

    Connections {
        target: Onboarding

        onUserPassFailed: {
            root.enabled = true;
            email.showType = true;
            password.showType = true;
            password.hintText = OnboardingStrings.errorLogin;
            password.hintVisible = true;
        }

        onLoginFinished: {
            root.enabled = true;
            onboardingFlow.state = syncs;
        }
    }

    Connections {
        target: password

        onTextChanged: {
            if(loginAttempt && !email.hintVisible && email.showType) {
                email.showType = false;
            }
        }
    }
}



