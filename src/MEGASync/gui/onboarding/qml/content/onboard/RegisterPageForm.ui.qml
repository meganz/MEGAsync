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
    readonly property int scrollbarWidth: 8
    readonly property int rightFormMargin: 13 + scrollbarWidth
    readonly property int scrollViewHeight: 374

    property alias firstName: firstName
    property alias lastName: lastName
    property alias firstLastNameHint: firstLastNameHint
    property alias email: email
    property alias password: password
    property alias confirmPassword: confirmPassword
    property alias termsCheckBox: termsCheckBox
    property alias dataLossCheckBox: dataLossCheckBox

    property alias loginButton: loginButton
    property alias nextButton: nextButton

    color: Styles.backgroundColor

    Column {
        anchors.left: root.left
        anchors.right: root.right
        anchors.top: root.top
        anchors.leftMargin: contentMargin
        anchors.rightMargin: contentMargin / 2
        anchors.topMargin: contentMargin
        spacing: contentMargin / 2

        Custom.RichText {
            id: title

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: email.textField.focusBorderWidth
            anchors.rightMargin: email.textField.focusBorderWidth
            font.pixelSize: 20
            text: OnboardingStrings.signUpTitle
        }

        ScrollView {
            id: scrollView

            anchors.left: parent.left
            anchors.right: parent.right
            height: root.scrollViewHeight
            clip: true

            ScrollBar.vertical: ScrollBar {
                id: scrollbar

                anchors.right: scrollView.right
                height: scrollView.height
                width: root.scrollbarWidth
                visible: formColumn.height > scrollView.height
                contentItem: Rectangle {
                    radius: 10
                    color: Styles.iconPrimary
                    opacity: scrollbar.pressed ? 0.6 : 1.0
                }
                background: Rectangle {
                    radius: 10
                    color: "#000000"
                    opacity: 0.1
                }
            }

            Column {
                id: formColumn

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: contentMargin / 2

                Column {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 12

                    Column {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        spacing: 4

                        Row {
                            id: nameLayout

                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.rightMargin: root.rightFormMargin
                            spacing: 8

                            Custom.TextField {
                                id: firstName

                                width: nameLayout.width / 2 - nameLayout.spacing / 2
                                title: OnboardingStrings.firstName
                                type: Custom.TextField.Type.Error
                            }

                            Custom.TextField {
                                id: lastName

                                width: nameLayout.width / 2 - nameLayout.spacing / 2
                                title: OnboardingStrings.lastName
                                type: Custom.TextField.Type.Error
                            }
                        }

                        Custom.HintText {
                            id: firstLastNameHint

                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.leftMargin: email.textField.focusBorderWidth
                            anchors.rightMargin: email.textField.focusBorderWidth
                            type: Custom.HintText.Type.Error
                            text: OnboardingStrings.errorFirstLastName
                        }
                    }

                    Custom.EmailTextField {
                        id: email

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.rightMargin: root.rightFormMargin
                        title: OnboardingStrings.email
                    }

                    Custom.PasswordTextField {
                        id: password

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.rightMargin: root.rightFormMargin
                        showHint: true
                        title: OnboardingStrings.password
                    }

                    Custom.PasswordTextField {
                        id: confirmPassword

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.rightMargin: root.rightFormMargin
                        title: OnboardingStrings.confirmPassword
                    }
                }

                Column {
                    id: checksLayout

                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 16

                    Custom.CheckBox {
                        id: dataLossCheckBox

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.rightMargin: root.rightFormMargin
                        url: Links.security
                        text: OnboardingStrings.understandLossPassword
                    }

                    Custom.CheckBox {
                        id: termsCheckBox

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.rightMargin: root.rightFormMargin
                        url: Links.terms
                        text: OnboardingStrings.agreeTerms
                    }
                }
            }
        }
    }

    Row {
        anchors.right: root.right
        anchors.bottom: root.bottom
        anchors.rightMargin: contentMargin
        anchors.bottomMargin: bottomMargin
        spacing: buttonSpacing

        Custom.Button {
            id: nextButton

            enabled: dataLossCheckBox.checked && termsCheckBox.checked
            primary: true
            iconSource: Images.arrowRight
            text: OnboardingStrings.next
        }

        Custom.Button {
            id: loginButton

            text: OnboardingStrings.login
        }
    }
}
