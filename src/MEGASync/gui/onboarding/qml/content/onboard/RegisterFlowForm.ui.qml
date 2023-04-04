import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Components 1.0 as Custom
import Common 1.0

Rectangle {
    id: root

    property alias loginPage: loginPage
    property alias registerPage: registerPage
    property alias twoFAPage: twoFAPage

    readonly property string login: "login"
    readonly property string twoFA: "twoFA"
    readonly property string register: "register"

    color: Styles.alternateBackgroundColor
    border.color: "#ffffff"

    states: [
        State {
            name: login
            PropertyChanges {
                target: loginPage
                visible: true
            }
        },
        State {
            name: register
            PropertyChanges {
                target: registerPage
                visible: true
            }
        },
        State {
            name: twoFA
            PropertyChanges {
                target: twoFAPage
                visible: true
            }
        }
    ]

    Image {
        id: image

        fillMode: Image.Tile
        source: "../../../../images/Onboarding/login_folder.png"
        anchors.left: root.left
        anchors.verticalCenter: root.verticalCenter
    }

    Rectangle {
        anchors {
            left: image.right
            top: root.top
            bottom: root.bottom
            right: root.right
        }

        LoginPage {
            id: loginPage

            visible: true
        }

        TwoFAPage {
            id: twoFAPage

            visible: false
        }

        RegisterPage {
            id: registerPage

            visible: false
        }
    }

}
