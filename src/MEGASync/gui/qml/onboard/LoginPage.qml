import QtQuick 2.15

import onboard 1.0

import ApiEnums 1.0
import LoginController 1.0
import AccountStatusController 1.0

LoginPageForm {
    id: root

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

    function resetLoginControllerStatus() {
        loginControllerAccess.emailError = false;
        loginControllerAccess.emailErrorMsg = "";
        loginControllerAccess.passwordError = false;
        loginControllerAccess.passwordErrorMsg = "";
    }

    state: {
        switch(loginControllerAccess.state) {
            case LoginController.LOGGING_IN:
                return stateInProgressLoggingIn;
            case LoginController.LOGGING_IN_2FA_REQUIRED:
            case LoginController.LOGGING_IN_2FA_VALIDATING:
            case LoginController.LOGGING_IN_2FA_FAILED:
                return state2FARequired;
            case LoginController.CREATING_ACCOUNT:
                return stateInProgressCreatingAccount;
            case LoginController.WAITING_EMAIL_CONFIRMATION:
                return stateInProgressWaitingEmailConfirm;
            case LoginController.FETCHING_NODES:
                return stateInProgressFetchNodes;
            case LoginController.FETCH_NODES_FINISHED:
                return stateFetchNodesFinished;
            case LoginController.FETCH_NODES_FINISHED_ONBOARDING:
                return stateFetchNodesFinishedOnboarding;
        }

        return stateLoggedOut;
    }

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
                creatingAccount: false;
            }
            PropertyChanges {
                target: root;
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
                creatingAccount: false;
            }
            PropertyChanges {
                target: root;
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
                creatingAccount: true;
            }
        },
        State {
            name: stateInProgressWaitingEmailConfirm
            extend: stateInProgress
            PropertyChanges {
                target: onboardingWindow
                loggingIn: false;
                creatingAccount: true;
            }
        },
        State {
            name: stateFetchNodesFinished
            extend: stateLoggedOut
            StateChangeScript {
                script: {
                    cancelLogin.close();
                    onboardingWindow.forceClose();
                }
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
        loginControllerAccess.loginError = ApiEnums.API_OK;
        loginControllerAccess.loginErrorMsg = "";
    }

    password.onTextChanged: {
        if(password.textField.text.length > 0) {
            loginControllerAccess.loginError = ApiEnums.API_OK;
            loginControllerAccess.loginErrorMsg = "";
        }
    }

    loginButton.onClicked: {
        var emailValid = email.valid();
        loginControllerAccess.emailError = !emailValid;
        var emailText = "";
        if(!emailValid) {
            emailText = email.text.length === 0
                        ? OnboardingStrings.errorEmptyEmail
                        : OnboardingStrings.errorValidEmail;
        }
        loginControllerAccess.emailErrorMsg = emailText;

        var passwordValid = password.text.length > 0;
        loginControllerAccess.passwordError = !passwordValid;
        loginControllerAccess.passwordErrorMsg = passwordValid ? "" : OnboardingStrings.errorEmptyPassword;

        if (!emailValid) {
            email.forceActiveFocus();
            return;
        }

        if (!passwordValid) {
            password.forceActiveFocus();
            return;
        }

        loginControllerAccess.login(email.text, password.text);
    }

    signUpButton.onClicked: {
        loginControllerAccess.state = LoginController.SIGN_UP;
    }

    Keys.onEnterPressed: {
        loginButton.forceActiveFocus();
        loginButton.clicked();
    }

    Keys.onReturnPressed: {
        loginButton.forceActiveFocus();
        loginButton.clicked();
    }

    Component.onDestruction: {
        resetLoginControllerStatus();
    }

    Component.onCompleted: {
        resetLoginControllerStatus();
    }

    Connections {
        target: accountStatusControllerAccess

        function onBlockedStateChanged(blockState) {
            if(blockState >= ApiEnums.ACCOUNT_BLOCKED_VERIFICATION_EMAIL) {
                cancelLogin.close();
                onboardingWindow.forceClose();
            }
        }
    }

    Connections {
        target: logoutControllerAccess

        function onLogout() {
            password.text = "";
            cancelLogin.close();
            onboardingWindow.forceClose();
        }
    }

    Connections {
        target: loginControllerAccess

        function onPasswordErrorChanged() {
            if(loginControllerAccess.passwordError) {
                password.focus  = true;
            }
        }
    }

    Connections {
        target: onboardingWindow

        function onInitializePageFocus() {
            if (loginControllerAccess.newAccount) {
                password.forceActiveFocus();
            }
            else {
                email.forceActiveFocus();
            }
        }
    }
}
