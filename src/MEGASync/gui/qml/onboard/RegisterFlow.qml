import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.dialogs 1.0

import LoginController 1.0

Rectangle {
    id: root

    readonly property string login: "login"
    readonly property string twoFA: "twoFA"
    readonly property string register: "register"
    readonly property string confirmEmail: "confirmEmail"
    readonly property string changeConfirmEmail: "changeConfirmEmail"

    color: colorStyle.surface1

    state:  {
        switch(loginControllerAccess.state) {
            case LoginController.WAITING_EMAIL_CONFIRMATION:
                return root.confirmEmail;
            case LoginController.SIGN_UP:
            case LoginController.CREATING_ACCOUNT:
            case LoginController.CREATING_ACCOUNT_FAILED:
                return root.register;
            case LoginController.LOGGING_IN_2FA_REQUIRED:
            case LoginController.LOGGING_IN_2FA_VALIDATING:
            case LoginController.LOGGING_IN_2FA_FAILED:
            case LoginController.FETCHING_NODES_2FA:
                return root.twoFA;
            case LoginController.CHANGING_REGISTER_EMAIL:
                return root.changeConfirmEmail;
        }

        return root.login;
    }

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

    // DO NOT REMOVE, windows qt bug. Without this line CancelLogin does not close when RegisterFlow is deleted
    Component.onDestruction: {}

    ConfirmCloseDialog {
        id: cancelLogin

        titleText: OnboardingStrings.cancelLoginTitle
        bodyText: OnboardingStrings.cancelLoginBodyText
        cancelButtonText: OnboardingStrings.cancelLoginSecondaryButton
        acceptButtonText: OnboardingStrings.cancelLoginPrimaryButton
        visible: false
        onAccepted: {
            loginControllerAccess.cancelLogin();
        }
    }

    ConfirmCloseDialog {
        id: cancelCreateAccount

        titleText: OnboardingStrings.cancelAccountCreationTitle
        bodyText: OnboardingStrings.cancelAccountCreationBody
        cancelButtonText: OnboardingStrings.cancelAccountCancelButton
        acceptButtonText: OnboardingStrings.cancelAccountAcceptButton
        visible: false
        onAccepted: {
            loginControllerAccess.cancelCreateAccount();
        }
    }

    Item {
        id: leftItem

        anchors {
            left: root.left
            top: root.top
            verticalCenter: root.verticalCenter
        }
        height: parent.height
        width: 304

        Image {
            id: leftImage

            anchors.centerIn: parent
            source: root.state === twoFA ? Images.twofa : Images.login

            onSourceChanged: {
                imageAnimation.start();
            }

            NumberAnimation on opacity {
                id: imageAnimation

                from: 0
                to: 1
                duration: 1000
            }
        }
    }

    Rectangle {
        id: separatorLine

        anchors {
            left: leftItem.right
            top: root.top
            topMargin: 48
        }
        width: 1
        height: 464
        radius: 2
        color: colorStyle.borderDisabled
    }

    StackViewBase {
        id: stack

        anchors {
            left: leftItem.right
            top: root.top
            bottom: root.bottom
            right: root.right
            margins: 48
            bottomMargin: 16
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
    }

    Connections {
        target: loginControllerAccess

        function onAccountCreationCancelled() {
            onboardingWindow.creatingAccount = false;
            cancelCreateAccount.close();
            onboardingWindow.forceClose();
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
}
