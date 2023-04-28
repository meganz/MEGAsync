// System
import QtQuick 2.12

// Local
import Components 1.0 as Custom
import Common 1.0
import Onboarding 1.0

Custom.TextField {

    property bool showHint: false

    enum PasswordStrength {
        PASSWORD_STRENGTH_VERYWEAK = 0,
        PASSWORD_STRENGTH_WEAK = 1,
        PASSWORD_STRENGTH_MEDIUM = 2,
        PASSWORD_STRENGTH_GOOD = 3,
        PASSWORD_STRENGTH_STRONG = 4
    }

    type: Custom.TextField.Type.Error

    textField.echoMode: TextInput.Password
    textField.onTextChanged: {
        rightIconVisible = (text.length > 0);
        hintVisible = text.length && showHint;
        showType = false;

        if(!showHint || !(text.length > 0)) {
            return;
        }

        var strength = Onboarding.getPasswordStrength(text);
        type = Custom.TextField.None;
        switch(strength) {
            case PasswordTextField.PasswordStrength.PASSWORD_STRENGTH_VERYWEAK: {
                hintTitle = qsTr("Too weak");
                hintText = qsTr("Your password needs to be at least 8 characters long.");
                hintTitleColor = Styles.textError;
                hintIconColor = Styles.indicatorPink;
                hintIconSource = Images.passwordVeryWeak;
                break;
            }
            case PasswordTextField.PasswordStrength.PASSWORD_STRENGTH_WEAK: {
                hintTitle = qsTr("Weak");
                hintText = qsTr("Your password is easily guessed. Try making your password longer. Combine uppercase and lowercase letters. Add special characters. Do not use names or dictionary words.");
                hintTitleColor = Styles.textError;
                hintIconColor = Styles.supportError;
                hintIconSource = Images.passwordWeak;
                break;
            }
            case PasswordTextField.PasswordStrength.PASSWORD_STRENGTH_MEDIUM: {
                hintTitle = qsTr("Average");
                hintText = qsTr("Your password is good enough to proceed, but it is recommended to strengthen it further.");
                hintTitleColor = Styles.supportWarning;
                hintIconColor = Styles.supportWarning;
                hintIconSource = Images.passwordAverage;
                break;
            }
            case PasswordTextField.PasswordStrength.PASSWORD_STRENGTH_GOOD: {
                hintTitle = qsTr("Strong");
                hintText = qsTr("This password will withstand most typical brute-force attacks. Please ensure that you will remember it.");
                hintTitleColor = Styles.textSuccess;
                hintIconColor = Styles.supportSuccess;
                hintIconSource = Images.passwordGood;
                break;
            }
            case PasswordTextField.PasswordStrength.PASSWORD_STRENGTH_STRONG: {
                hintTitle = qsTr("Excellent");
                hintText = qsTr("This password will withstand most sophisticated brute-force attacks. Please ensure that you will remember it.");
                hintTitleColor = Styles.supportSuccess;
                hintIconColor = Styles.supportSuccess;
                hintIconSource = Images.passwordStrong;
                break;
            }
        }
    }

    rightIconVisible: false
    rightIconSource: "images/eye.svg"
    rightIconMouseArea.onClicked: {
        textField.focus = true;
        if(textField.echoMode === TextInput.Password) {
            textField.echoMode = TextInput.Normal;
            rightIconSource = "images/eye-off.svg";
        } else if(textField.echoMode === TextInput.Normal) {
            textField.echoMode = TextInput.Password;
            rightIconSource = "images/eye.svg";
        }
    }
}
