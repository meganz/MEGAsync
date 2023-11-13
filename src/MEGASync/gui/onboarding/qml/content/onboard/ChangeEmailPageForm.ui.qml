// System
import QtQuick 2.15
import QtQuick.Layouts 1.15

// QML common
import Common 1.0
import Components.Buttons 1.0 as MegaButtons
import Components.Texts 1.0 as MegaTexts
import Components.TextFields 1.0 as MegaTextFields

// Local
import Onboard 1.0

StackViewPage {
    id: root

    readonly property int buttonsLayoutSpacing: 2

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
            id: bodyText

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
        id: buttonsLayout

        anchors {
            right: root.right
            bottom: root.bottom
            rightMargin: -resendButton.sizes.focusBorderWidth
            bottomMargin: buttonsBottomMargin
        }
        spacing: buttonsLayoutSpacing

        MegaButtons.OutlineButton {
            id: cancelButton

            text: OnboardingStrings.cancel
        }

        MegaButtons.PrimaryButton {
            id: resendButton

            text: OnboardingStrings.resend
            icons.source: Images.mail
        }
    }
}
