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

StackViewPage {
    id: root

    property alias emailTextField: emailTextField
    property alias cancelButton: cancelButton
    property alias resendButton: resendButton

    ColumnLayout {
        id: layout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: contentSpacing

        MegaTexts.Text {
            id: title

            Layout.fillWidth: true
            text: OnboardingStrings.changeEmailTitle
            font.pixelSize: MegaTexts.Text.Size.Large

        }

        MegaTexts.Text {
            Layout.preferredWidth: layout.width
            text: OnboardingStrings.changeEmailBodyText
            font.pixelSize: MegaTexts.Text.Size.Medium
        }

        MegaTextFields.EmailTextField {
            id: emailTextField

            title: OnboardingStrings.email
            Layout.preferredWidth: layout.width + 2 * emailTextField.sizes.focusBorderWidth
            Layout.leftMargin: -emailTextField.sizes.focusBorderWidth
        }
    }

    RowLayout {

        anchors {
            bottom: parent.bottom
            right: parent.right
        }
        spacing: 8

        MegaButtons.OutlineButton {
            id: cancelButton

            text: OnboardingStrings.cancel
        }

        MegaButtons.PrimaryButton {
            id: resendButton

            Layout.rightMargin: -resendButton.sizes.focusBorderWidth
            text: OnboardingStrings.resend
            icons.source: Images.mail
        }
    }
}
