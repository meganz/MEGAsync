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

    property alias firstName: firstName
    property alias lastName: lastName
    property alias firstLastNameDescription: firstLastNameDescription
    property alias email: email
    property alias password: password
    property alias confirmPassword: confirmPassword

    property alias loginButton: loginButton
    property alias nextButton: nextButton

    property int contentMargin: 48

    color: Styles.backgroundColor

    Custom.RichText {
        id: title

        anchors {
            left: root.left
            top: root.top
            leftMargin: contentMargin + 4
            topMargin: contentMargin
        }

        font.pixelSize: 20
        text: OnboardingStrings.signUpTitle
        Layout.leftMargin: 4
    }

    ScrollView {
        id: scrollView

        anchors {
            left: root.left
            top: title.bottom
            leftMargin: contentMargin
            topMargin: contentMargin / 2
            rightMargin: contentMargin
        }

        width: formLayout.width + 24
        height: 380
        clip : true

        ScrollBar.vertical: ScrollBar {
            id: scrollbar

            anchors.right: scrollView.right
            height: scrollView.height
            width: 8
            visible: formLayout.height > scrollView.height
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

        ColumnLayout {
            id: formLayout

            width: root.width - 2 * contentMargin + 2 * email.textField.focusBorderWidth
            spacing: contentMargin / 2

            ColumnLayout {
                Layout.preferredWidth: formLayout.width
                spacing: 12

                ColumnLayout {
                    spacing: 4

                    RowLayout {
                        id: nameLayout

                        spacing: 8
                        Layout.preferredWidth: formLayout.width

                        Custom.TextField {
                            id: firstName

                            title: OnboardingStrings.firstName
                            Layout.preferredWidth: (nameLayout.width - 8) / 2
                            type: Custom.TextField.Type.Error
                        }

                        Custom.TextField {
                            id: lastName

                            title: OnboardingStrings.lastName
                            Layout.preferredWidth: (nameLayout.width - 8) / 2
                            type: Custom.TextField.Type.Error
                        }
                    }

                    RowLayout {
                        id: firstLastNameDescription

                        width: formLayout.width
                        visible: false
                        spacing: 8
                        Layout.leftMargin: 4

                        Custom.SvgImage {
                            source: Images.alertTriangle
                            sourceSize: Qt.size(16, 16)
                            color: Styles.textError
                        }

                        Text {
                            Layout.fillWidth: true
                            text: OnboardingStrings.errorFirstLastName
                            color: Styles.textError
                            wrapMode: Text.WordWrap
                            font {
                                pixelSize: 12
                                weight: Font.Light
                                family: "Inter"
                                styleName: "Medium"
                            }
                        }
                    }
                }

                Custom.EmailTextField {
                    id: email

                    title: OnboardingStrings.email
                    Layout.preferredWidth: formLayout.width
                }

                Custom.PasswordTextField {
                    id: password

                    title: OnboardingStrings.password
                    Layout.preferredWidth: formLayout.width
                }

                Custom.PasswordTextField {
                    id: confirmPassword

                    title: OnboardingStrings.confirmPassword
                    Layout.preferredWidth: formLayout.width
                }

            }

            ColumnLayout {
                id: checksLayout

                Layout.preferredWidth: formLayout.width - 2 * email.textField.focusBorderWidth
                Layout.leftMargin: 4
                spacing: 16

                Custom.CheckBox {
                    id: dataLossCheckBox

                    Layout.preferredWidth: checksLayout.width
                    url: Links.security
                    text: OnboardingStrings.understandLossPassword
                }

                Custom.CheckBox {
                    id: termsCheckBox

                    Layout.preferredWidth: checksLayout.width
                    Layout.bottomMargin: 5
                    url: Links.terms
                    text: OnboardingStrings.agreeTerms
                }
            }

        }
    }

    RowLayout {
        id: buttonLayout

        spacing: 8

        anchors {
            right: root.right
            bottom: root.bottom
            bottomMargin: 32
            rightMargin: contentMargin
        }

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
