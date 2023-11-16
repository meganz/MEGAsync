// System
import QtQuick 2.15

// C++
import LoginController 1.0

TwoFAPageForm {
    id: root

    readonly property string validating2FA: "validating"
    readonly property string fetchingNodes2FA: "fetchingNodes2FA"
    readonly property string validating2FAFailed: "validating2FAFailed"
    readonly property string normal: "normal"

    state: {
        switch(loginControllerAccess.state) {
            case LoginController.LOGGING_IN_2FA_VALIDATING:
                return validating2FA;
            case LoginController.FETCHING_NODES_2FA:
                return fetchingNodes2FA;
            case LoginController.LOGGING_IN_2FA_FAILED:
                return validating2FAFailed;
            default:
                return normal;
        }
    }

    states: [
        State {
            name: normal
            PropertyChanges {
                target: root
                enabled: true
            }
            PropertyChanges {
                target: loginButton
                icons.busyIndicatorVisible: false
            }
        },
        State {
            name: validating2FA
            PropertyChanges {
                target: root
                enabled: false
            }
            PropertyChanges {
                target: loginButton
                icons.busyIndicatorVisible: true
            }
        },
        State {
            name: validating2FAFailed
            extend: normal
            PropertyChanges {
                target: twoFAField
                hasError: true
            }
        },
        State {
            name: fetchingNodes2FA
            extend: validating2FA
        }
    ]

    signUpButton.onClicked: {
        loginControllerAccess.state = LoginController.SIGN_UP;
        loginControllerAccess.email = "";
    }

    loginButton.onClicked: {
        loginControllerAccess.login2FA(twoFAField.key);
    }

    twoFAField.onAllDigitsFilled: {
        loginControllerAccess.login2FA(twoFAField.key);
    }

    Connections {
        target: onboardingWindow

        function onInitializePageFocus() {
            twoFAField.forceFocus()
        }
    }
}
