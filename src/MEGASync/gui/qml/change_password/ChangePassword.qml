import QtQuick 2.0

import common 1.0

import components.views 1.0

import ChangePasswordComponents 1.0

ChangePasswordDialog {
    id: window

    title: ChangePasswordStrings.title
    visible: true
    modality: Qt.WindowModal
    width: 496
    height: 320
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width

    Rectangle {
        id: changePasswordContentItem

        anchors.fill: parent
        color: ColorTheme.surface1

        readonly property string change_password: "change_password"
        readonly property string two_fa: "two_fa"

        state: change_password
        states: [
            State {
                name: changePasswordContentItem.change_password
                StateChangeScript {
                    script: stackView.replace(changePasswordPage);
                }
            },
            State {
                name: changePasswordContentItem.two_fa
                StateChangeScript {
                    script: stackView.replace(twoFAPage);
                }
            }
        ]

        StackViewBase {
            id: stackView

            anchors {
                fill: parent
                margins: 48
            }

            Component {
                id: changePasswordPage

                ChangePasswordPage {
                    id: changePasswordItem
                }
            }

            Component {
                id: twoFAPage

                Rectangle{

                }

                /*
                TwoFAPage {
                    id: twoFAItem

                    footerButtons.leftPrimary.visible: false
                }
                */
            }
        }
    }
}
