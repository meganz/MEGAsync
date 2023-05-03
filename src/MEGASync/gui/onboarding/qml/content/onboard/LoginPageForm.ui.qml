// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components 1.0 as Custom
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

    color: Styles.backgroundColor

    Column {
        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.right: root.right
        anchors.leftMargin: contentMargin
        anchors.rightMargin: contentMargin
        spacing: contentMargin / 2

        Custom.RichText {
            id: pageTitle

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: email.textField.focusBorderWidth
            anchors.rightMargin: email.textField.focusBorderWidth
            font.pixelSize: 20
            text: OnboardingStrings.loginTitle
        }

        Custom.EmailTextField {
            id: email

            anchors.left: parent.left
            anchors.right: parent.right
            title: OnboardingStrings.email
        }

        Custom.PasswordTextField {
            id: password

            anchors.left: parent.left
            anchors.right: parent.right
            title: OnboardingStrings.password
        }

        Custom.HelpButton {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: email.textField.focusBorderWidth
            anchors.rightMargin: email.textField.focusBorderWidth
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

        Custom.OutlineButton {
            id: signUpButton

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
        }

        Custom.PrimaryButton {
            id: loginButton

            text: OnboardingStrings.login
            busyIndicatorImage: Images.loader
            iconSource: Images.arrowRight
            Layout.alignment: Qt.AlignRight
            progressBar: true
        }
    }
}
