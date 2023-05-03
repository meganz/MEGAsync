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

    color: Styles.pageBackground

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
            font.pixelSize: 20
            text: OnboardingStrings.loginTitle
        }

        Custom.EmailTextField {
            id: email

            width: parent.width + 2 * focusWidth
            anchors.left: parent.left
            anchors.leftMargin: -focusWidth
            title: OnboardingStrings.email
        }

        Custom.PasswordTextField {
            id: password

            width: parent.width + 2 * focusWidth
            anchors.left: parent.left
            anchors.leftMargin: -focusWidth
            title: OnboardingStrings.password
        }

        Custom.HelpButton {
            anchors.left: parent.left
            anchors.right: parent.right
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
