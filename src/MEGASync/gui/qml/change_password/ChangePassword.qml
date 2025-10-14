import QtQuick 2.0

import common 1.0

import components.views 1.0

import ChangePasswordComponents 1.0

import components.textFields 1.0

ChangePasswordDialog {
    id: window

    readonly property int twoFAPageHeigh: 500
    readonly property int passwordChangePageHeigh: 320
    readonly property int passwordChangePageWidth: 496

    title: ChangePasswordStrings.title
    visible: true
    modality: Qt.WindowModal
    width: passwordChangePageWidth
    height: passwordChangePageHeigh
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
                PropertyChanges { target: window; height: twoFAPageHeigh; }
                PropertyChanges { target: window; maximumHeight: twoFAPageHeigh; }
                PropertyChanges { target: window; minimumHeight: twoFAPageHeigh; }
            }
        ]

        StackViewBase {
            id: stackView

            anchors {
                fill: parent
                margins: Constants.bigWindowMargin
            }

            Component {
                id: changePasswordPage

                ChangePasswordPage {
                    id: changePasswordItem
                }
            }

            Component {
                id: twoFAPage

                TwoFAPage {
                    id: twoFAItem
                }
            }
        }

        Connections{
            id: changePassConn

            target: changePasswordComponentAccess

            function onShow2FA() {
                changePasswordContentItem.state = changePasswordContentItem.two_fa;
            }

            function onPasswordChangeSucceed() {
                window.close();
            }
        }
    }
}
