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

Rectangle {
    id: root

    readonly property int contentMargin: 48
    readonly property int bottomMargin: 32
    readonly property int buttonSpacing: 8

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

    Row {
        anchors.right: root.right
        anchors.bottom: root.bottom
        anchors.rightMargin: contentMargin
        anchors.bottomMargin: bottomMargin
        spacing: buttonSpacing

        Custom.Button {
            id: loginButton

            primary: true
            text: OnboardingStrings.login
        }

        Custom.Button {
            id: signUpButton

            text: OnboardingStrings.signUp
        }
    }

}
