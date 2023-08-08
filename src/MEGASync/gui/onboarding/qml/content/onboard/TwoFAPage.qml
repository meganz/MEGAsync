// System
import QtQuick 2.12
import QtQuick.Controls 2.12

TwoFAPageForm {

    signUpButton.onClicked: {
        registerFlow.state = register;
    }

    loginButton.onClicked: {
        state = code2FAStatus;
        loginButton.icons.busyIndicatorVisible = true;
        LoginControllerAccess.login2FA(twoFAField.key);
    }

    Connections {
        target: LoginControllerAccess

        onFetchingNodesProgress: {
            loginButton.progress.value = progress;
        }

        onFetchingNodesFinished: {
            onboardingWindow.loggingIn = false;
            if(firstTime)
            {
                loginButton.icons.busyIndicatorVisible = false;
                onboardingFlow.state = syncs;
            }
            else
            {
                onboardingWindow.close();
            }
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
            case -5: //mega::MegaError::API_EFAILED: -> 2FA failed
            case -8: //mega::MegaError::API_EEXPIRED: -> 2FA failed (expired)
            {
                twoFAField.hasError = true;
                loginButton.icons.busyIndicatorVisible = false;
                state = normalStatus;
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
}
