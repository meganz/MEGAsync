import QtQuick 2.15

import common 1.0

import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.pages 1.0
import components.textFields 1.0
import onboard 1.0

FooterButtonsPage {
    id: root

    readonly property int minimumAllowedPasswordLength: 8
    readonly property int mainColumnDesignSpacing: 24

    function error() {
        var error = false;

        var valid = passwordItem.text.length >= minimumAllowedPasswordLength;
        if (!valid) {
            error = true;
            passwordItem.error = true;
            passwordItem.hint.text = OnboardingStrings.minimum8Chars;
        }
        passwordItem.error = !valid;
        passwordItem.hint.visible = !valid;

        if (confirmPasswordItem.text.length === 0) {
            error = true;
            confirmPasswordItem.error = true;
            confirmPasswordItem.hint.visible = true;
            confirmPasswordItem.hint.text = OnboardingStrings.errorConfirmPassword;
        }
        else if (passwordItem.text !== confirmPasswordItem.text) {
            error = true;
            confirmPasswordItem.error = true;
            confirmPasswordItem.hint.visible = true;
            confirmPasswordItem.hint.text = OnboardingStrings.errorPasswordsMatch;
            passwordItem.hint.visible = false;
            passwordItem.error = true;
        }
        else {
            confirmPasswordItem.error = false;
            confirmPasswordItem.hint.visible = false;
        }

        return error;
    }

    footerButtons {
        leftPrimary.visible: false
        rightSecondary {
            visible: true
            text: Strings.cancel

            onClicked: {
                window.close();
            }
        }

        rightPrimary {
            text: ChangePasswordStrings.title
            icons: Icon {}

            enabled: passwordItem.validPassword && confirmPasswordItem.text !== ""

            onClicked: {
                if (error()) {
                    return;
                }

                changePasswordComponentAccess.changePassword(passwordItem.text, confirmPasswordItem.text);
            }
        }
    }

    Column {
        id: mainFormLayout

        anchors.fill: parent
        spacing: mainColumnDesignSpacing

        PasswordTextField {
            id: passwordItem

            property bool validPassword: passwordItem.textField.text.length >= minimumAllowedPasswordLength
                                            && passChecker.validPassword

            width: parent.width
            title: ChangePasswordStrings.newPassword
            cleanWhenError: false

            textField.onActiveFocusChanged: {
                if (textField.activeFocus) {
                    hint.visible = false;
                }
                else {
                    var hintVisible = true;
                    if (textField.text.length < minimumAllowedPasswordLength) {
                        hint.text = OnboardingStrings.minimum8Chars;
                        hint.textColor = ColorTheme.textError;
                    }
                    else {
                        if (!passChecker.validPassword) {
                            hint.text = OnboardingStrings.passwordEasilyGuessedError;
                            hint.textColor = ColorTheme.textError;
                        }
                        else if (!passChecker.allChecked) {
                            hint.text = OnboardingStrings.passwordEasilyGuessed;
                            hint.textColor = ColorTheme.textWarning;
                        }
                        else {
                            hintVisible = false;
                        }
                    }
                    hint.visible = hintVisible;
                }
            }

            PasswordChecker {
                id: passChecker

                password: passwordItem.textField.text
            }

        }

        PasswordTextField {
            id: confirmPasswordItem

            width: parent.width
            title: ChangePasswordStrings.confirmNewPassword
            hint.icon: Images.key
            cleanWhenError: false
        }
    }

}


