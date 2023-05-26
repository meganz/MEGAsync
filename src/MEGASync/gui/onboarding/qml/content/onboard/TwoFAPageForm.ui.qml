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

    readonly property int contentMargin: 48
    readonly property int bottomMargin: 32

    color: Styles.pageBackground

    ColumnLayout {

        spacing: 24
        anchors {
            verticalCenter: root.verticalCenter
            left: root.left
            right: root.right
            leftMargin: 48
            rightMargin: 48
        }

        Header {
            title: OnboardingStrings.twoFATitle
            description: OnboardingStrings.twoFASubtitle
        }

        MegaTextFields.TwoFA {
            id: twoFAField

            Layout.leftMargin: -3
            Layout.preferredWidth: parent.width
            Layout.fillHeight: true
        }

        MegaButtons.HelpButton {
            text: OnboardingStrings.twoFANeedHelp
            url: Links.recovery
        }
    }

    RowLayout {
        spacing: 8
        anchors {
            right: root.right
            bottom: root.bottom
            left: root.left
            leftMargin: contentMargin
            rightMargin: contentMargin
            bottomMargin: bottomMargin
        }

        MegaButtons.OutlineButton {
            id: signUpButton

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
        }

        MegaButtons.PrimaryButton {
            id: loginButton

            text: OnboardingStrings.login
            Layout.alignment: Qt.AlignRight
            icons.source: Images.arrowRight
        }
    }
}
