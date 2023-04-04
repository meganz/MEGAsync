import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Components 1.0 as Custom
import Common 1.0
import Onboarding 1.0

Rectangle {
    id: registerFlow

    readonly property string login: "login"
    readonly property string twoFA: "twoFA"
    readonly property string register: "register"

    color: Styles.alternateBackgroundColor
    border.color: "#ffffff"

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
        }
    ]

    Image {
        id: image

        fillMode: Image.Tile
        source: "../../../../images/Onboarding/login_folder.png"
        anchors.left: registerFlow.left
        anchors.verticalCenter: registerFlow.verticalCenter
        z: 2
    }

    StackView {
        id: stack

        anchors {
            left: image.right
            top: registerFlow.top
            bottom: registerFlow.bottom
            right: registerFlow.right
        }

        LoginPage {
            id: loginPage

            visible: false

            createAccountButton.onClicked: {
                registerFlow.state = register;
            }
        }

        TwoFAPage {
            id: twoFAPage

            visible: false
        }

        RegisterPage {
            id: registerPage

            visible: false

            loginButton.onClicked: {
                registerFlow.state = login;
            }
        }
    }

    Connections{
        target: Onboarding

        onTwoFARequired: {
            registerFlow.state = twoFA;
        }
    }
}
