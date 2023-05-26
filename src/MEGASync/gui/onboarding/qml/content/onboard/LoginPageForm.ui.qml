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

    readonly property int contentMargin: 48
    readonly property int bottomMargin: 32

    property alias signUpButton: signUpButton
    property alias loginButton: loginButton

    property alias email: email
    property alias password: password

    color: Styles.pageBackground

    Column {
        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.right: root.right
        anchors.leftMargin: contentMargin
        anchors.rightMargin: contentMargin
        spacing: contentMargin / 2

        MegaTexts.RichText {
            id: pageTitle

            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: MegaTexts.Text.Size.Large
            text: OnboardingStrings.loginTitle
        }

        MegaTextFields.EmailTextField {
            id: email

            width: parent.width
            anchors.left: parent.left
            title: OnboardingStrings.email
        }

        MegaTextFields.PasswordTextField {
            id: password

            width: parent.width
            anchors.left: parent.left
            title: OnboardingStrings.password
        }

        MegaButtons.HelpButton {
            anchors.left: parent.left
            anchors.right: parent.right
            text: OnboardingStrings.forgotPassword
            url: Links.recovery
        }
    }

    RowLayout {
        anchors {
            right: root.right
            bottom: root.bottom
            left: root.left
            leftMargin: contentMargin
            rightMargin: contentMargin
            bottomMargin: bottomMargin
        }

        MegaButtons.SecondaryButton {
            id: signUpButton

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
        }

        MegaButtons.PrimaryButton {
            id: loginButton

            text: OnboardingStrings.login
            icons.source: Images.arrowRight
            Layout.alignment: Qt.AlignRight
        }
    }
}
