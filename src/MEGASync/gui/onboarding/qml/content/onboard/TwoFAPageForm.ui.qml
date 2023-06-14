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

StackViewPage {
    id: root

    property alias loginButton: loginButton
    property alias signUpButton: signUpButton
    property alias twoFAField: twoFAField

    ColumnLayout {

        spacing: contentSpacing
        anchors {
            verticalCenter: root.verticalCenter
            left: root.left
            right: root.right
        }

        Header {
            title: OnboardingStrings.twoFATitle
            description: OnboardingStrings.twoFASubtitle
        }

        MegaTextFields.TwoFA {
            id: twoFAField
        }

        MegaButtons.HelpButton {
            text: OnboardingStrings.twoFANeedHelp
            url: Links.recovery
        }
    }

    RowLayout {
        anchors {
            right: root.right
            bottom: root.bottom
            left: root.left
        }

        MegaButtons.OutlineButton {
            id: signUpButton

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: -signUpButton.focusBorderWidth
        }

        MegaButtons.PrimaryButton {
            id: loginButton

            text: OnboardingStrings.login
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: -loginButton.focusBorderWidth
            icons.source: Images.arrowRight
        }
    }
}
