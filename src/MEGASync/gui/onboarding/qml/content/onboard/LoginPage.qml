// System
import QtQuick 2.12

// Local
import Onboard 1.0

// C++
import Onboarding 1.0
import ApiEnums 1.0
import LoginController 1.0
import AccountStatusController 1.0

LoginPageForm {
    id: loginPage

    property bool loginAttempt: false

    readonly property string stateLoggedOut: "LOGGED_OUT"
    readonly property string stateInProgress: "IN_PROGRESS"
    readonly property string stateInProgressLoggingIn: "IN_PROGRESS_LOGGING"
    readonly property string stateInProgress2FA: "IN_PROGRESS_2FA"
    readonly property string state2FARequired: "2FA_REQUIRED"
    readonly property string stateInProgressFetchNodes: "IN_PROGRESS_FETCH_NODES"
    readonly property string stateInProgressCreatingAccount: "CREATING_ACCOUNT"
    readonly property string stateInProgressWaitingEmailConfirm: "WAITING_EMAIL_CONFIRMATION"
    readonly property string stateFetchNodesFinished: "FETCH_NODES_FINISHED"
    readonly property string stateFetchNodesFinishedOnboarding: "FETCH_NODES_FINISHED_ONBOARDING"

    function getState(){
            switch(LoginControllerAccess.state)
            {
            case LoginController.LOGGING_IN:
            {
                return stateInProgressLoggingIn;
            }
            case LoginController.LOGGING_IN_2FA_REQUIRED:
            case LoginController.LOGGING_IN_2FA_VALIDATING:
            case LoginController.LOGGING_IN_2FA_FAILED:
            {
                return state2FARequired;
            }
            case LoginController.CREATING_ACCOUNT:
            {
                return stateInProgressCreatingAccount;
            }
            case LoginController.WAITING_EMAIL_CONFIRMATION:
            {
                return stateInProgressWaitingEmailConfirm;
            }
            case LoginController.FETCHING_NODES:
            {
                return stateInProgressFetchNodes;
            }
            case LoginController.FETCH_NODES_FINISHED:
            {
                return stateFetchNodesFinished;
            }
            case LoginController.FETCH_NODES_FINISHED_ONBOARDING:
            {
                return stateFetchNodesFinishedOnboarding;
            }
            }
            return stateLoggedOut;
    }

    state: getState();
    states: [
        State {
            name: stateLoggedOut
            PropertyChanges {
                target: loginButton;
                icons.busyIndicatorVisible: false;
                progressValue: 0;
            }
            PropertyChanges {
                target: onboardingWindow
                loggingIn: false;
            }
            PropertyChanges {
                target: loginPage;
                enabled: true;
            }
        },
        State {
            name: stateInProgress
            PropertyChanges {
                target: loginButton
                icons.busyIndicatorVisible: true;
            }
            PropertyChanges {
                target: onboardingWindow
                loggingIn: true;
            }
            PropertyChanges {
                target: loginPage;
                enabled: false;
            }
        },
        State {
            name: stateInProgressLoggingIn
            extend: stateInProgress
        },
        State {
            name: stateInProgressFetchNodes
            extend: stateInProgress
        },
        State {
            name: stateInProgressCreatingAccount
            extend: stateInProgress
            PropertyChanges {
                target: onboardingWindow
                loggingIn: false;
            }
        },
        State {
            name: stateInProgressWaitingEmailConfirm
            extend: stateInProgress
            PropertyChanges {
                target: onboardingWindow
                loggingIn: false;
            }
        },
        State {
            name: stateFetchNodesFinished
            extend: stateLoggedOut
            StateChangeScript {
                script: {
                        cancelLogin.close();
                        onboardingWindow.forceClose();}
                        }
        },
        State {
            name: stateFetchNodesFinishedOnboarding
            extend: stateLoggedOut
            PropertyChanges {
                target: onboardingFlow
                state: syncs
            }
        }
    ]

    email.onTextChanged: {
        LoginControllerAccess.loginError = ApiEnums.API_OK;
        LoginControllerAccess.loginErrorMsg = "";
    }

    password.onTextChanged: {
        if(password.textField.text.length > 0) {
            LoginControllerAccess.loginError = ApiEnums.API_OK;
            LoginControllerAccess.loginErrorMsg = "";
        }
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
        var emailValid = email.valid();
        LoginControllerAccess.emailError = !emailValid;
        var emailText = "";
        if(!emailValid) {
            emailText = email.text.length === 0
                        ? OnboardingStrings.errorEmptyEmail
                        : OnboardingStrings.errorValidEmail;
        }
        LoginControllerAccess.emailErrorMsg = emailText;

        var passwordValid = password.text.length > 0;
        LoginControllerAccess.passwordError = !passwordValid;
        LoginControllerAccess.passwordErrorMsg = passwordValid ? "" : OnboardingStrings.errorEmptyPassword;

        if(!emailValid || !passwordValid) {
            return;
        }

        LoginControllerAccess.login(email.text, password.text);
        loginAttempt = true;
    }

    signUpButton.onClicked: {
        LoginControllerAccess.state = LoginController.SIGN_UP;
    }

    Connections {
        target: AccountStatusControllerAccess

        function onBlockedStateChanged() {
            if(blockState >= ApiEnums.ACCOUNT_BLOCKED_VERIFICATION_SMS)
            {
                cancelLogin.close();
                onboardingWindow.forceClose();
            }
        }
    }

    Connections {
        target: LogoutControllerAccess

        function onLogout() {
            password.text = "";
            cancelLogin.close();
            onboardingWindow.forceClose();
        }
    }

    Component.onDestruction:
    {
        resetLoginControllerStatus()
    }

    Component.onCompleted:
    {
        resetLoginControllerStatus()
    }

    function resetLoginControllerStatus()
    {
        LoginControllerAccess.emailError = false;
        LoginControllerAccess.emailErrorMsg = "";
        LoginControllerAccess.passwordError = false;
        LoginControllerAccess.passwordErrorMsg = "";
    }
}
