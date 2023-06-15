// System
import QtQuick 2.12

// Local
import Components.Texts 1.0 as MegaTexts
import Components.TextFields 1.0 as MegaTextFields
import Common 1.0
import Onboarding 1.0

MegaTextFields.TextField {

    property bool showHint: false

    enum PasswordStrength {
        PasswordStrengthVeryWeak,
        PasswordStrengthWeak,
        PasswordStrengthMedium,
        PasswordStrengthGood,
        PasswordStrengthStrong
    }

    textField.echoMode: TextInput.Password

    readonly property MegaTexts.HintStyle veryWeak: MegaTexts.HintStyle {
        iconColor: Styles.indicatorPink
        titleColor: Styles.textError
        textColor: Styles.textSecondary
    }

    readonly property MegaTexts.HintStyle weak: MegaTexts.HintStyle {
        iconColor: Styles.supportError;
        titleColor: Styles.textError;
        textColor: Styles.textSecondary;
    }

    readonly property MegaTexts.HintStyle medium: MegaTexts.HintStyle {
        iconColor: Styles.supportWarning;
        titleColor: Styles.supportWarning;
        textColor: Styles.textSecondary;
    }

    readonly property MegaTexts.HintStyle good: MegaTexts.HintStyle {
        iconColor: Styles.supportSuccess;
        titleColor: Styles.textSuccess;
        textColor: Styles.textSecondary;
    }

    readonly property MegaTexts.HintStyle strong: MegaTexts.HintStyle {
        iconColor: Styles.supportSuccess;
        titleColor: Styles.supportSuccess;
        textColor: Styles.textSecondary;
    }

    textField.onTextChanged: {
        rightIcon.visible = (text.length > 0);
        error = false;
        hint.visible = text.length > 0 && showHint;
        if(!(text.length > 0) || !showHint) {
            return;
        }

        var strength = Onboarding.getPasswordStrength(text);
        switch(strength) {
            case PasswordTextField.PasswordStrength.PasswordStrengthVeryWeak: {
                hint.title = qsTr("Too weak");
                hint.text = qsTr("Your password needs to be at least 8 characters long.");
                hint.styles = veryWeak;
                hint.icon = Images.passwordVeryWeak
                break;
            }
            case PasswordTextField.PasswordStrength.PasswordStrengthWeak: {
                hint.title = qsTr("Weak");
                hint.text = qsTr("Your password is easily guessed. Try making your password longer. Combine uppercase and lowercase letters. Add special characters. Do not use names or dictionary words.");
                hint.styles = weak;
                hint.icon = Images.passwordWeak;
                break;
            }
            case PasswordTextField.PasswordStrength.PasswordStrengthMedium: {
                hint.title = qsTr("Average");
                hint.text = qsTr("Your password is good enough to proceed, but it is recommended to strengthen it further.");
                hint.styles = medium;
                hint.icon = Images.passwordAverage;
                break;
            }
            case PasswordTextField.PasswordStrength.PasswordStrengthGood: {
                hint.title = qsTr("Strong");
                hint.text = qsTr("This password will withstand most typical brute-force attacks. Please ensure that you will remember it.");
                hint.styles = good;
                hint.icon = Images.passwordGood;
                break;
            }
            case PasswordTextField.PasswordStrength.PasswordStrengthStrong: {
                hint.title = qsTr("Excellent");
                hint.text = qsTr("This password will withstand most sophisticated brute-force attacks. Please ensure that you will remember it.");
                hint.styles = strong;
                hint.icon = Images.passwordStrong;
                break;
            }
        }
    }

    textField.onFocusChanged: {
        if(textField.focus) {
            rightIcon.visible = true;
        }
    }

    rightIcon.visible: false
    rightIcon.source: "images/eye.svg"
    rightIconMouseArea.onClicked: {
        textField.focus = true;
        if(textField.echoMode === TextInput.Password) {
            textField.echoMode = TextInput.Normal;
            rightIcon.source = "images/eye-off.svg";
        } else if(textField.echoMode === TextInput.Normal) {
            textField.echoMode = TextInput.Password;
            rightIcon.source = "images/eye.svg";
        }
    }
}
