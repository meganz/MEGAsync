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

    property alias signUpButton: signUpButton
    property alias loginButton: loginButton

    property alias email: email
    property alias password: password

    color: Styles.backgroundColor

    ColumnLayout {
        id: mainLayout

        spacing: 24
        anchors {
            verticalCenter: root.verticalCenter
            left: root.left
            right: root.right
            leftMargin: 48
            rightMargin: 48
        }

        Custom.RichText {
            font.pixelSize: 20
            text: OnboardingStrings.loginTitle
            Layout.bottomMargin: textFieldsLayout.spacing
            Layout.leftMargin: 4
        }

        ColumnLayout {
            id: textFieldsLayout

            spacing: 12
            Layout.preferredWidth: mainLayout.width

            Custom.EmailTextField {
                id: email

                title: OnboardingStrings.email
                Layout.preferredWidth: textFieldsLayout.width
            }

            Custom.PasswordTextField {
                id: password

                title: OnboardingStrings.password
                Layout.preferredWidth: textFieldsLayout.width
            }
        }

        Custom.HelpButton {
            text: OnboardingStrings.forgotPassword
            url: Links.recovery
            Layout.leftMargin: 4
        }
    }

    RowLayout {
        spacing: 8
        anchors {
            right: root.right
            bottom: root.bottom
            rightMargin: 48
            bottomMargin: 32
        }

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

    Connections {
        target: Onboarding

        onUserPassFailed: {
            email.descriptionType = Custom.TextField.DescriptionType.Error;
            password.descriptionType = Custom.TextField.DescriptionType.Error;
            password.descriptionText = OnboardingStrings.errorLogin;
        }
    }
}
