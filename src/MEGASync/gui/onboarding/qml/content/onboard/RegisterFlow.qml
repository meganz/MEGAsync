// System
import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components 1.0 as Custom

// C++
import Onboarding 1.0

Rectangle {
    id: registerFlow

    readonly property string login: "login"
    readonly property string twoFA: "twoFA"
    readonly property string register: "register"
    readonly property string confirmEmail: "confirmEmail"
    readonly property string changeConfirmEmail: "changeConfirmEmail"

    color: "transparent"
    state: login
    states: [
        State {
            name: login
            StateChangeScript {
                script: stack.replace(loginPage);
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
                script: stack.replace(registerPage);
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

    Rectangle {
        anchors.fill: registerFlow
        layer.smooth: true
        layer.enabled: true
        color: Styles.surface1
        opacity: 0.9
    }

    Image {
        id: leftImage

        source: registerFlow.state === twoFA ? Images.twofa : Images.login
        anchors.left: registerFlow.left
        anchors.verticalCenter: registerFlow.verticalCenter

        NumberAnimation on opacity {
            id: imageAimation

            from: 0
            to: 1
            duration: 1000
        }

        onSourceChanged: {
            imageAimation.start();
        }
    }

    StackView {
        id: stack

        anchors {
            left: leftImage.right
            top: registerFlow.top
            bottom: registerFlow.bottom
            right: registerFlow.right
        }
        replaceEnter: Transition {
            XAnimator {
                from: (stack.mirrored ? -1 : 1) * leftImage.width
                to: 0
                duration: 400
            }
        }
        replaceExit: Transition {
            XAnimator {
                from: 0
                to: (stack.mirrored ? -1 : 1) * stack.width
                duration: 400
            }
        }

        LoginPage {
            id: loginPage

            width: stack.width
            height: stack.height
            visible: false
        }

        TwoFAPage {
            id: twoFAPage

            visible: false
        }

        RegisterPage {
            id: registerPage

            visible: false
        }

        ConfirmEmailPage {
            id: confirmEmailPage

            visible: false
        }

        ChangeEmailPage {
            id: changeConfirmEmailPage

            visible: false
        }
    }

    Connections {
        target: Onboarding

        onTwoFARequired: {
            registerFlow.state = twoFA;
        }
    }
}
