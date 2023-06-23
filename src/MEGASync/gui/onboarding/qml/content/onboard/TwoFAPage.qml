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
        loginController.login2FA(twoFAField.key);
    }

    Connections {
        target: loginController

        onFetchingNodesProgress: {
            loginButton.progress.value = progress;
        }

        onFetchingNodesFinished: {
            loginButton.icons.busyIndicatorVisible = false;
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
