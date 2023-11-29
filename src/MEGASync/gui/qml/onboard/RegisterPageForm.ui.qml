import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0
import components.texts 1.0 as Texts

import LoginController 1.0

StackViewPage {
    id: root

    property alias registerContent: registerContentItem
    property alias loginButton: loginButtonItem
    property alias nextButton: nextButtonItem

    enabled: loginControllerAccess.state !== LoginController.CREATING_ACCOUNT

    Column {
        id: mainColumn

        anchors {
            left: root.left
            right: root.right
            top: root.top
        }
        spacing: contentSpacing

        Texts.RichText {
            anchors {
                left: parent.left
                right: parent.right
            }
            font.pixelSize: Texts.Text.Size.Large
            rawText: OnboardingStrings.signUpTitle
        }

        RegisterContent {
            id: registerContentItem

            anchors {
                left: parent.left
                right: parent.right
            }
        }
    }

    RowLayout {
        id: buttonsLayout

        anchors {
            left: root.left
            right: root.right
            bottom: root.bottom
            leftMargin: -loginButtonItem.sizes.focusBorderWidth
            rightMargin: -loginButtonItem.sizes.focusBorderWidth
            bottomMargin: buttonsBottomMargin
        }

        OutlineButton {
            id: loginButtonItem

            text: OnboardingStrings.login
            Layout.alignment: Qt.AlignLeft
        }

        PrimaryButton {
            id: nextButtonItem

            enabled: registerContentItem.firstName.text !== ""
                        && registerContentItem.lastName.text !== ""
                        && registerContentItem.email.text !== ""
                        && registerContentItem.password.validPassword
                        && registerContentItem.confirmPassword.text !== ""
                        && registerContentItem.termsCheckBox.checked
            text: OnboardingStrings.next
            icons {
                source: Images.arrowRight
                busyIndicatorVisible: loginControllerAccess.state === LoginController.CREATING_ACCOUNT
            }
            Layout.alignment: Qt.AlignRight
        }
    }
}
