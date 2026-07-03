import QtQuick 2.15

import common 1.0

import components.views 1.0
import components.textFields 1.0

import ChangePasswordComponents 1.0

ChangePasswordDialog {
    id: window

    readonly property int twoFAPageHeigh: 500
    readonly property int passwordChangePageHeigh: 344
    readonly property int passwordChangePageWidth: 496

    title: ChangePasswordStrings.title
    visible: false
    modality: Qt.WindowModal
    color: ColorTheme.pageBackground
    width: passwordChangePageWidth
    height: passwordChangePageHeigh
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width

    Rectangle {
        id: changePasswordContentItem

        anchors.fill: parent
        color: ColorTheme.pageBackground

        readonly property string change_password: "change_password"
        readonly property string two_fa: "two_fa"

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
                    script: {
                        // Grow the window before switching page. maximumHeight is
                        // raised before height so setHeight is not clamped by the
                        // stale passwordChangePageHeigh maximum, which would otherwise
                        // leave the window at 344 until a geometry event (e.g. moving
                        // the dialog) re-applied the constraints.
                        window.maximumHeight = twoFAPageHeigh;
                        window.height = twoFAPageHeigh;
                        window.minimumHeight = twoFAPageHeigh;
                        stackView.replace(twoFAPage);
                    }
                }
            }
        ]

        StackViewBase {
            id: stackView

            anchors {
                fill: parent
                margins: Constants.bigWindowMargin
            }

            initialItem: changePasswordPage

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
