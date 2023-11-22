import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0
import components.texts 1.0 as Texts
import components.textFields 1.0

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

        Texts.Text {
            id: title

            Layout.fillWidth: true
            text: OnboardingStrings.changeEmailTitle
            font.pixelSize: Texts.Text.Size.Large
        }

        Texts.Text {
            id: bodyText

            Layout.preferredWidth: layout.width
            text: OnboardingStrings.changeEmailBodyText
            font.pixelSize: Texts.Text.Size.Medium
        }

        EmailTextField {
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

        OutlineButton {
            id: cancelButton

            text: OnboardingStrings.cancel
        }

        PrimaryButton {
            id: resendButton

            text: OnboardingStrings.resend
            icons.source: Images.mail
        }
    }
}
