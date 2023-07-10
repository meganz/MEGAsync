// System
import QtQuick 2.12
// Local
import Onboarding 1.0
import ApiEnums 1.0

// C++
import Onboard 1.0
import LoginController 1.0


LoginPageForm {
    id: root

    property bool loginAttempt: false
    property bool twoFARequired: false

    function setNormalStatus(){
        root.enabled = true;
        loginButton.icons.busyIndicatorVisible = false;
        state = normalStatus;
        password.text = "";
        onboardingWindow.loggingIn = false;
    }

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
        target: Onboarding

        onAccountBlocked:
        {
            setNormalStatus();
            onboardingWindow.forceClose();
        }
    }

    Connections {
        target: loginController

        onFetchingNodesProgress: {
            console.log("LOGIN PAGE progress:"+progress)
            loginButton.progress.value = progress;
        }

        onFetchingNodesFinished: {
            onboardingWindow.loggingIn = false;
            if(firstTime)
            {
                loginButton.icons.busyIndicatorVisible = false;
                state = normalStatus;
                onboardingFlow.state = syncs;
            }
            else
            {
                onboardingWindow.close();
            }
        }

        onAccountCreationResumed: {
            registerFlow.state = confirmEmail;
        }

        onLogoutBySdk: {
            onboardingWindow.forceClose();
            setNormalStatus();
        }

        onLogoutByUser: {
            cancelLogin.close();
            onboardingWindow.forceClose();
            setNormalStatus();
        }

        onLoginFinished: {
            if(errorCode !== ApiEnums.API_OK)
            {
                setNormalStatus();
            }

            switch(errorCode)
            {
                case ApiEnums.API_EMFAREQUIRED://-26: //mega::MegaError::API_EMFAREQUIRED:->2FA required
                {
                    registerFlow.state = twoFA;
                    break;
                }
                case ApiEnums.API_EFAILED: //mega::MegaError::API_EFAILED: ->
                case ApiEnums.API_EEXPIRED: //mega::MegaError::API_EEXPIRED: -> 2FA failed
                {
                    break;
                }
                case ApiEnums.API_ENOENT: //mega::MegaError::API_ENOENT: -> user or pass failed
                {
                    email.error = true;
                    password.error = true;
                    password.hint.text = OnboardingStrings.errorLogin;
                    password.hint.visible = true;
                    break;
                }
                case ApiEnums.API_EINCOMPLETE: //mega::MegaError::API_EINCOMPLETE: -> account not confirmed
                {
                    //what to do here?
                    break;
                }
                case ApiEnums.API_ETOOMANY: //mega::MegaError::API_ETOOMANY: -> too many attempts
                {
                    //what to do here?
                    break;
                }
                case ApiEnums.API_EBLOCKED: //mega::MegaError::API_EBLOCKED: ->  blocked account
                {
                    //what to do here?
                    break;
                }
                case ApiEnums.API_OK: //mega::MegaError::API_OK:
                {
                    state = fetchNodesStatus;
                    break;
                }
                default:

            }
        }
    }
}
