// System
import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// QML common
import Common 1.0

// C++
import Onboarding 1.0
import LoginController 1.0

Rectangle {
    id: registerFlow

    readonly property string login: "login"
    readonly property string twoFA: "twoFA"
    readonly property string register: "register"
    readonly property string confirmEmail: "confirmEmail"
    readonly property string changeConfirmEmail: "changeConfirmEmail"

    color: Styles.surface1

    function getState() {
        switch(loginControllerAccess.state)
        {
            case LoginController.WAITING_EMAIL_CONFIRMATION:
            {
                console.log("Waiting for email confirmation")
                return registerFlow.confirmEmail;
            }
            case LoginController.SIGN_UP:
            case LoginController.CREATING_ACCOUNT:
            case LoginController.CREATING_ACCOUNT_FAILED:
            {
                console.log("Creating account")
                return registerFlow.register;
            }
            case LoginController.LOGGING_IN_2FA_REQUIRED:
            case LoginController.LOGGING_IN_2FA_VALIDATING:
            case LoginController.LOGGING_IN_2FA_FAILED:
            case LoginController.FETCHING_NODES_2FA:
            {
                console.log("Logging in 2FA")
                return registerFlow.twoFA;
            }
            case LoginController.CHANGING_REGISTER_EMAIL:
            {
                console.log("CHANGING_REGISTER_EMAIL")
                return registerFlow.changeConfirmEmail;
            }
        }

        console.log("registerFlow.login")
        return registerFlow.login;
    }
    state: getState();
    states: [
        State {
            name: login
            StateChangeScript {
                script: {
                    stack.replace(loginPage);
                }
            }
        },
        State {
            name: twoFA
            StateChangeScript {
                script: stack.replace(twoFAPage);
            }
        },
        State {
            name: register
            StateChangeScript {
                script: {
                    stack.replace(registerPage);
                }
            }
        },
        State {
            name: confirmEmail
            StateChangeScript {
                script: stack.replace(confirmEmailPage);
            }
        },
        State {
            name: changeConfirmEmail
            StateChangeScript {
                script: stack.replace(changeConfirmEmailPage);
            }
        }
    ]

    CancelLogin {
        id: cancelLogin

        visible: false
        onAccepted: {
            loginControllerAccess.cancelLogin();
        }
    }

    CancelCreateAccount {
        id: cancelCreateAccount

        visible: false
        onAccepted: {
            loginControllerAccess.cancelCreateAccount();
        }
    }

    Connections {
        target: onboardingWindow

        function onClosingButLoggingIn() {
            cancelLogin.visible = true;
        }

        function onClosingButCreatingAccount() {
            cancelCreateAccount.visible = true;
        }
    }

    Image {
        id: leftImage

        source: registerFlow.state === twoFA ? Images.twofa : Images.login
        anchors.left: registerFlow.left
        anchors.leftMargin: 2
        anchors.verticalCenter: registerFlow.verticalCenter
        width: 300

        NumberAnimation on opacity {
            id: imageAnimation

            from: 0
            to: 1
            duration: 1000
        }

        onSourceChanged: {
            imageAnimation.start();
        }
    }

    Rectangle {
        id: separatorLine

        anchors {
            left: leftImage.right
            top: registerFlow.top
            leftMargin: 2
            topMargin: 48
        }
        width: 1
        height: 464
        radius: 2
        color: Styles.borderDisabled
    }

    StackView {
        id: stack

        anchors {
            left: separatorLine.right
            top: registerFlow.top
            bottom: registerFlow.bottom
            right: registerFlow.right
            leftMargin: 48
            rightMargin: 48
            topMargin: 48
            bottomMargin: 16
        }

        replaceEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to:1
                duration: 100
                easing.type: Easing.OutQuad
            }
        }
        replaceExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to:0
                duration: 100
                easing.type: Easing.InQuad
            }
        }

        onCurrentItemChanged: {
            currentItem.setInitialFocusPosition();
        }

        Component {
            id: loginPage

            LoginPage {}
        }

        Component {
            id: twoFAPage

            TwoFAPage {}
        }

        Component {
            id: registerPage

            RegisterPage {}
        }

        Component {
            id: confirmEmailPage

            ConfirmEmailPage {}
        }

        Component {
            id: changeConfirmEmailPage

            ChangeEmailPage {}
        }

        Connections {
            target: loginControllerAccess

            function onAccountCreationCancelled() {
                onboardingWindow.creatingAccount = false;
                cancelCreateAccount.close();
                onboardingWindow.forceClose();
            }
        }
    }

    //DO NOT REMOVE, windows qt bug. Without this line CancelLogin does not close when RegisterFlow is deleted
    Component.onDestruction: {
    }
}
