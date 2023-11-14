// System
import QtQuick 2.15
import QtQuick.Layouts 1.15

// QML common
import Common 1.0
import Components.Buttons 1.0 as MegaButtons
import Components.TextFields 1.0 as MegaTextFields

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

// C++
import LoginController 1.0

StackViewPage {
    id: root

    property alias loginButton: loginButtonItem
    property alias signUpButton: signUpButtonItem
    property alias twoFAField: twoFAItem

    ColumnLayout {
        id: mainColumn

        anchors {
            left: root.left
            right: root.right
            verticalCenter: root.verticalCenter
        }
        spacing: contentSpacing

        Header {
            id: headerItem

            title: OnboardingStrings.twoFATitle
            description: OnboardingStrings.twoFASubtitle
        }

        MegaTextFields.TwoFA {
            id: twoFAItem

            focus: true
        }

        MegaButtons.HelpButton {
            id: helpButtonItem

            text: OnboardingStrings.twoFANeedHelp
            url: Links.recovery
            visible: !twoFAItem.hasError
        }
    }

    RowLayout {
        id: buttonLayout

        anchors {
            left: root.left
            right: root.right
            bottom: root.bottom
            leftMargin: -signUpButtonItem.sizes.focusBorderWidth
            rightMargin: -signUpButtonItem.sizes.focusBorderWidth
            bottomMargin: buttonsBottomMargin
        }

        MegaButtons.OutlineButton {
            id: signUpButtonItem

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
        }

        MegaButtons.PrimaryButton {
            id: loginButtonItem

            text: OnboardingStrings.login
            progressValue: loginControllerAccess.progress
            Layout.alignment: Qt.AlignRight
        }
    }

}
