// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components.Buttons 1.0 as MegaButtons
import Components.Texts 1.0 as MegaTexts
import Components.TextFields 1.0 as MegaTextFields
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0
import LoginController 1.0
import ApiEnums 1.0

StackViewPage {
    id: root

    property alias signUpButton: signUpButton
    property alias loginButton: loginButton

    property alias email: email
    property alias password: password

    Column {
        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.right: root.right
        spacing: contentSpacing

        MegaTexts.RichText {
            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: MegaTexts.Text.Size.Large
            rawText: LoginControllerAccess.newAccount
                     ? OnboardingStrings.confirmEmailAndPassword
                     : OnboardingStrings.loginTitle
        }

        MegaTexts.RichText {
            visible: LoginControllerAccess.newAccount
            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: MegaTexts.Text.Size.Medium
            rawText: OnboardingStrings.accountWillBeActivated
        }

        MegaTextFields.EmailTextField {
            id: email

            width: parent.width + 2 * email.sizes.focusBorderWidth
            anchors.left: parent.left
            anchors.leftMargin: -email.sizes.focusBorderWidth
            title: OnboardingStrings.email
            text: LoginControllerAccess.email
            error: LoginControllerAccess.emailError
            hint.text: LoginControllerAccess.emailErrorMsg
            hint.visible: LoginControllerAccess.emailErrorMsg.length !== 0
        }

        MegaTextFields.PasswordTextField {
            id: password

            width: parent.width + 2 * password.sizes.focusBorderWidth
            anchors.left: parent.left
            anchors.leftMargin: -password.sizes.focusBorderWidth
            title: OnboardingStrings.password
            hint.icon: Images.alertTriangle
            error: LoginControllerAccess.passwordError
            hint.text: LoginControllerAccess.passwordErrorMsg
            hint.visible: LoginControllerAccess.passwordErrorMsg.length !== 0
        }

        MegaButtons.HelpButton {
            anchors.left: parent.left
            text: OnboardingStrings.forgotPassword
            url: Links.recovery
            visible: !LoginControllerAccess.newAccount
        }
    }

    RowLayout {
        anchors.right: root.right
        anchors.bottom: root.bottom
        anchors.bottomMargin: 29
        anchors.left: root.left

        MegaButtons.OutlineButton {
            id: signUpButton

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: -signUpButton.sizes.focusBorderWidth
            visible: !LoginControllerAccess.newAccount
        }

        MegaButtons.PrimaryButton {
            id: loginButton

            text: LoginControllerAccess.newAccount ? OnboardingStrings.next : OnboardingStrings.login
            Layout.alignment: Qt.AlignRight
            progressValue: LoginControllerAccess.progress
            Layout.rightMargin: -loginButton.sizes.focusBorderWidth//TODO: poner flecha
            icons.source: LoginControllerAccess.newAccount ? Images.arrowRight : Images.none
        }
    }
}
