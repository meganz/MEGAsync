import QtQuick 2.0

import common 1.0

import components.views 1.0

import ChangePasswordComponents 1.0

ChangePasswordDialog {
    id: window

    title: ChangePasswordStrings.syncsWindowTitle
    visible: true
    modality: Qt.NonModal
    width: 640
    height: 620
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width

    Column {
        id: contentItem

        anchors.fill: parent

        Rectangle {
            id: changePasswordContentItem

            width: parent.width
            height: parent.height - stepPanelItem.height
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
                    margins: Constants.defaultWindowMargin
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
}
