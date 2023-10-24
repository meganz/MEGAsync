// System
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

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
    id: root

    property alias registerContent: registerContentItem
    property alias loginButton: loginButtonItem
    property alias nextButton: nextButtonItem

    enabled: LoginControllerAccess.state !== LoginController.CREATING_ACCOUNT;

    Column {
        id: mainColumn

        anchors.left: root.left
        anchors.right: root.right
        anchors.top: root.top

        spacing: contentSpacing

        MegaTexts.RichText {
            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: MegaTexts.Text.Size.Large
            rawText: OnboardingStrings.signUpTitle
        }

        RegisterContent {
            id: registerContentItem

            anchors.left: parent.left
            anchors.right: parent.right
        }
    }

    RowLayout {
        anchors {
            right: root.right
            bottom: root.bottom
            bottomMargin: 29
            left: root.left
        }

        MegaButtons.OutlineButton {
            id: loginButtonItem

            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: -loginButtonItem.sizes.focusBorderWidth
            text: OnboardingStrings.login
        }

        MegaButtons.PrimaryButton {
            id: nextButtonItem

            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: -loginButtonItem.sizes.focusBorderWidth
            enabled: registerContentItem.firstName.text !== ""
                        && registerContentItem.lastName.text !== ""
                        && registerContentItem.email.text !== ""
                        && registerContentItem.password.validPassword
                        && registerContentItem.confirmPassword.text !== ""
                        && registerContentItem.termsCheckBox.checked
            icons.source: Images.arrowRight
            text: OnboardingStrings.next
            icons.busyIndicatorVisible: LoginControllerAccess.state === LoginController.CREATING_ACCOUNT;
        }
    }
}
