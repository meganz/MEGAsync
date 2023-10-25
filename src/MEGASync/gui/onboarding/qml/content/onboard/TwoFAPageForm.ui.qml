// System
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

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

    property alias loginButton: loginButtonItem
    property alias signUpButton: signUpButtonItem
    property alias twoFAField: twoFAItem

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
            id: twoFAItem

            focus: true
        }

        MegaButtons.HelpButton {
            text: OnboardingStrings.twoFANeedHelp
            url: Links.recovery
            visible: !twoFAItem.hasError
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
            id: signUpButtonItem

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: -signUpButtonItem.sizes.focusBorderWidth
        }

        MegaButtons.PrimaryButton {
            id: loginButtonItem

            text: OnboardingStrings.login
            progressValue: loginControllerAccess.progress
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: -loginButtonItem.sizes.focusBorderWidth
        }
    }

}
