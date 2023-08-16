// System
import QtQuick 2.12
import QtQuick.Controls 2.12

// C++
import ApiEnums 1.0
import LoginController 1.0

TwoFAPageForm {

    signUpButton.onClicked: {
        registerFlow.state = register;
        LoginControllerAccess.cancelLogin2FA();
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
            if(firstTime) {
                loginButton.icons.busyIndicatorVisible = false;
                onboardingFlow.state = syncs;
            } else {
                onboardingWindow.close();
            }
        }

        onLoginFinished: {
            switch(errorCode) {
                case ApiEnums.API_EMFAREQUIRED: //mega::MegaError::API_EMFAREQUIRED:->2FA required
                    loginButton.icons.busyIndicatorVisible = false;
                    break;
                case ApiEnums.API_EFAILED: //mega::MegaError::API_EFAILED: ->
                case ApiEnums.API_EEXPIRED: //mega::MegaError::API_EEXPIRED: -> 2FA failed
                    twoFAField.hasError = true;
                    loginButton.icons.busyIndicatorVisible = false;
                    state = normalStatus;
                    break;
                case ApiEnums.API_ETOOMANY: //mega::MegaError::API_ETOOMANY: -> too many attempts
                    //what to do here?
                    break;
                case ApiEnums.API_EBLOCKED: //mega::MegaError::API_EBLOCKED: ->  blocked account
                    //what to do here?                    //add banners
                    break;
                case ApiEnums.API_OK: //mega::MegaError::API_OK:
                    state = fetchNodesStatus;
                    break;
                default:
                    break;
            }
        }
    }
}
