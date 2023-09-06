// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components.Buttons 1.0 as MegaButtons
import Components.Texts 1.0 as MegaTexts
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0
import LoginController 1.0

StackViewPage {
    id: rootRegisterPage

    property alias registerContent: registerContent
    property alias loginButton: loginButton
    property alias nextButton: nextButton

    enabled: LoginControllerAccess.state !== LoginController.CREATING_ACCOUNT;

    Column {
        id: mainColumn

        anchors.left: rootRegisterPage.left
        anchors.right: rootRegisterPage.right
        anchors.top: rootRegisterPage.top

        spacing: contentSpacing

        MegaTexts.RichText {
            id: title

            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: MegaTexts.Text.Size.Large
            text: OnboardingStrings.signUpTitle
        }

        RegisterContent {
            id: registerContent

            anchors.left: parent.left
            anchors.right: parent.right
        }
    }

    RowLayout {
        anchors {
            right: rootRegisterPage.right
            bottom: rootRegisterPage.bottom
            bottomMargin: 29
            left: rootRegisterPage.left
        }

        MegaButtons.OutlineButton {
            id: loginButton

            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: -loginButton.sizes.focusBorderWidth
            text: OnboardingStrings.login
        }

        MegaButtons.PrimaryButton {
            id: nextButton

            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: -loginButton.sizes.focusBorderWidth
            enabled: registerContent.firstName.text !== ""
                        && registerContent.lastName.text !== ""
                        && registerContent.email.text !== ""
                        && registerContent.password.validPassword
                        && registerContent.confirmPassword.text !== ""
                        && registerContent.termsCheckBox.checked
            icons.source: Images.arrowRight
            text: OnboardingStrings.next
            icons.busyIndicatorVisible: LoginControllerAccess.state === LoginController.CREATING_ACCOUNT;
        }
    }
}
