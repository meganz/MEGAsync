// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components.Buttons 1.0 as MegaButtons
import Components.TextFields 1.0 as MegaTextFields
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0
import ApiEnums 1.0
import LoginController 1.0

StackViewPage {
    id: root

    property alias loginButton: loginButton
    property alias signUpButton: signUpButton
    property alias twoFAField: twoFAField

    ColumnLayout {

        spacing: contentSpacing
        anchors {
            top: root.top
            left: root.left
            right: root.right
            topMargin: 115
        }

        Header {
            title: OnboardingStrings.twoFATitle
            description: OnboardingStrings.twoFASubtitle
        }

        MegaTextFields.TwoFA {
            id: twoFAField

            focus: true
        }

        MegaButtons.HelpButton {
            text: OnboardingStrings.twoFANeedHelp
            url: Links.recovery
            visible: !twoFAField.hasError
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
            id: signUpButton

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: -signUpButton.sizes.focusBorderWidth
        }

        MegaButtons.PrimaryButton {
            id: loginButton

            text: OnboardingStrings.login
            progressValue: LoginControllerAccess.progress
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: -loginButton.sizes.focusBorderWidth
        }
    }

}
