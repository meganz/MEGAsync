// System
import QtQuick 2.12

// Local
import Onboarding 1.0

// C++
import Onboard 1.0

LoginPageForm {
    id: loginPageRoot

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
        loginController.login(email.text, password.text);
        onboardingWindow.loggingIn = true;
        loginAttempt = true;
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
        target: loginController

        onFetchingNodesProgress: {
            console.log("LOGIN PAGE progress:"+progress)
            loginButton.progress.value = progress;
        }

        onFetchingNodesFinished: {
            loginButton.icons.busyIndicatorVisible = false;
            state = normalStatus;
            onboardingFlow.state = syncs;
            onboardingWindow.loggingIn = false;
        }

        onLoginFinished: {
            switch(errorCode)
            {
                case -26: //mega::MegaError::API_EMFAREQUIRED:->2FA required
                {
                    loginButton.icons.busyIndicatorVisible = false;
                    registerFlow.state = twoFA;
                    break;
                }
                case -5: //mega::MegaError::API_EFAILED: ->
                case -8: //mega::MegaError::API_EEXPIRED: -> 2FA failed
                {

                    break;
                }
                case -9: //mega::MegaError::API_ENOENT: -> user or pass failed
                {
                    root.enabled = true;
                    email.error = true;
                    password.error = true;
                    password.hint.text = OnboardingStrings.errorLogin;
                    password.hint.visible = true;
                    loginButton.icons.busyIndicatorVisible = false;
                    state = normalStatus;
                    break;
                }
                case -13: //mega::MegaError::API_EINCOMPLETE: -> account not confirmed
                {
                    //what to do here?
                    break;
                }
                case -6: //mega::MegaError::API_ETOOMANY: -> too many attempts
                {
                    //what to do here?
                    break;
                }
                case -16: //mega::MegaError::API_EBLOCKED: ->  blocked account
                {
                    //what to do here?
                    break;
                }
                case 0: //mega::MegaError::API_OK:
                {
                    state = fetchNodesStatus;
                    break;
                }
            }
        }
    }

    Component.onCompleted: {
        if(email.text.length && password.text.length)
        {
            loginPageRoot.loginButton.clicked();
        }
    }
}
