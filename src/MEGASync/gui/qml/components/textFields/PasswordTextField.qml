// System
import QtQuick 2.12

// Local
import Components 1.0 as Custom
import Common 1.0
import Onboarding 1.0

Custom.TextField {

    property bool showHint: false

    enum PasswordStrength {
        PasswordStrengthVeryWeak,
        PasswordStrengthWeak,
        PasswordStrengthMedium,
        PasswordStrengthGood,
        PasswordStrengthStrong
    }

    textField.echoMode: TextInput.Password

    function getCoso()
    {
        return Custom.HintPasswordVeryWeakStyle;
    }

    readonly property Custom.HintStyle veryWeak: Custom.HintStyle{
        iconColor: Styles.indicatorPink
        titleColor: Styles.textError
        textColor: Styles.textSecondary
    }

    readonly property Custom.HintStyle weak: Custom.HintStyle{
        iconColor: Styles.supportError;
        titleColor: Styles.textError;
        textColor: Styles.textSecondary;
    }

    readonly property Custom.HintStyle medium: Custom.HintStyle{
        iconColor: Styles.supportWarning;
        titleColor: Styles.supportWarning;
        textColor: Styles.textSecondary;
    }

    readonly property Custom.HintStyle good: Custom.HintStyle{
        iconColor: Styles.supportSuccess;
        titleColor: Styles.textSuccess;
        textColor: Styles.textSecondary;
    }

    readonly property Custom.HintStyle strong: Custom.HintStyle{
        iconColor: Styles.supportSuccess;
        titleColor: Styles.supportSuccess;
        textColor: Styles.textSecondary;
    }

    textField.onTextChanged: {
        rightIcon.visible = (text.length > 0);
        error = false;

        if(!(text.length > 0)) {
            return;
        }
        else
        {
            hint.visible = true;
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
