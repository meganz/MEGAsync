import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0
import components.texts 1.0 as Texts
import components.textFields 1.0 as TextFields

import LoginController 1.0

StackViewPage {
    id: root

    property alias signUpButton: signUpButtonItem
    property alias loginButton: loginButtonItem
    property alias email: emailItem
    property alias password: passwordItem

    Column {
        id: mainColumn

        anchors {
            left: root.left
            right: root.right
            top: root.top
            topMargin: 120
        }
        spacing: contentSpacing

        Texts.RichText {
            id: mainTitle

            anchors {
                left: parent.left
                right: parent.right
            }
            font.pixelSize: Texts.Text.Size.Large
            rawText: loginControllerAccess.newAccount
                     ? OnboardingStrings.confirmEmailAndPassword
                     : OnboardingStrings.loginTitle
        }

        Texts.RichText {
            id: accountWillBeActivatedText

            anchors {
                left: parent.left
                right: parent.right
            }
            visible: loginControllerAccess.newAccount
            font.pixelSize: Texts.Text.Size.Medium
            rawText: OnboardingStrings.accountWillBeActivated
        }

        Column {
            id: inputsLayout

            anchors {
                left: parent.left
                leftMargin: -emailItem.sizes.focusBorderWidth
            }
            width: parent.width + 2 * emailItem.sizes.focusBorderWidth
            spacing: contentSpacing / 2

            TextFields.EmailTextField {
                id: emailItem

                width: parent.width
                title: OnboardingStrings.email
                text: loginControllerAccess.email
                error: loginControllerAccess.emailError
                hint {
                    text: loginControllerAccess.emailErrorMsg
                    visible: loginControllerAccess.emailErrorMsg.length !== 0
                }
            }

            TextFields.PasswordTextField {
                id: passwordItem

                width: parent.width
                title: OnboardingStrings.password
                error: loginControllerAccess.passwordError
                hint {
                    icon: Images.alertTriangle
                    text: loginControllerAccess.passwordErrorMsg
                    visible: loginControllerAccess.passwordErrorMsg.length !== 0
                }
            }
        }
    }

    LinkButton {
        id: helpButtonItem

        anchors.top: mainColumn.bottom
        anchors.left: root.left
        anchors.leftMargin: -sizes.horizontalAlignWidth
        anchors.topMargin: contentSpacing - sizes.verticalAlignWidth
        text: OnboardingStrings.forgotPassword
        url: Links.recovery
        icons {
            source: Images.helpCircle
            position: Icon.Position.LEFT
        }
        visible: !loginControllerAccess.newAccount
        sizes: SmallSizes { isLinkOrTextButton: true }
    }

    RowLayout {
        id: buttonsLayout

        anchors {
            left: root.left
            right: root.right
            bottom: root.bottom
            leftMargin: -signUpButtonItem.sizes.focusBorderWidth
            rightMargin: -signUpButtonItem.sizes.focusBorderWidth
            bottomMargin: buttonsBottomMargin
        }

        OutlineButton {
            id: signUpButtonItem

            text: OnboardingStrings.signUp
            visible: !loginControllerAccess.newAccount
            Layout.alignment: Qt.AlignLeft
        }

        PrimaryButton {
            id: loginButtonItem

            text: loginControllerAccess.newAccount ? OnboardingStrings.next : OnboardingStrings.login
            progressValue: loginControllerAccess.progress
            icons.source: loginControllerAccess.newAccount ? Images.arrowRight : Images.none
            Layout.alignment: Qt.AlignRight
        }
    }
}
