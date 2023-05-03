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

        loginButton.busyIndicatorVisible = true;
        state = logInStatus;
        loginButton.animationDuration = 2000;
        loginButton.progressValue = 0.8;
        Onboarding.onLoginClicked({ [Onboarding.RegisterForm.EMAIL]: email.text,
                                    [Onboarding.RegisterForm.PASSWORD]: password.text })
        loginAttempt = true;
    }

    loginButton.onAnimationFinished: {
        if(completed)
        {
            loginButton.busyIndicatorVisible = false;
            state = normalStatus;
            if(twoFARequired)
            {
                registerFlow.state = twoFA;
            }
            else
            {
                onboardingFlow.state = syncs;
                loginButton.progressValue = 0;
            }

        }
    }

    signUpButton.onClicked: {
        registerFlow.state = register;
    }

    password.onTextChanged: {
        if(loginAttempt && !email.hintVisible && email.showType) {
            email.showType = false;
        }
    }

    Connections {
        target: Onboarding

        onUserPassFailed: {
            root.enabled = true;
            email.showType = true;
            password.showType = true;
            password.hintText = OnboardingStrings.errorLogin;
            password.hintVisible = true;
            loginButton.progressValue = 0
            loginButton.busyIndicatorVisible = false;
            state = normalStatus;
        }

        onFetchingNodesProgress: {
            console.log("LOGIN PAGE progress:"+progress)
            loginButton.progressValue = progress;
        }

        onLoginFinished: {
            loginButton.progressValue = 0; //start fetching nodes
            loginButton.animationDuration = 1000;
            state = fetchNodesStatus;
        }

        onTwoFARequired: {
            twoFARequired = true;
            loginButton.progressValue = 1;
        }
    }
}




/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
