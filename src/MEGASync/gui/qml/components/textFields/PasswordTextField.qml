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

    type: Custom.TextField.Type.Error
    hintType: Custom.HintText.Type.Error

    textField.echoMode: TextInput.Password
    textField.onTextChanged: {
        rightIconVisible = (text.length > 0);
        hintVisible = text.length && showHint;
        showType = false;

        if(!showHint || !(text.length > 0)) {
            return;
        }

        var strength = Onboarding.getPasswordStrength(text);
        switch(strength) {
            case PasswordTextField.PasswordStrength.PasswordStrengthVeryWeak: {
                hintTitle = qsTr("Too weak");
                hintText = qsTr("Your password needs to be at least 8 characters long.");
                hintType = Custom.HintText.Type.PasswordStrengthVeryWeak;
                break;
            }
            case PasswordTextField.PasswordStrength.PasswordStrengthWeak: {
                hintTitle = qsTr("Weak");
                hintText = qsTr("Your password is easily guessed. Try making your password longer. Combine uppercase and lowercase letters. Add special characters. Do not use names or dictionary words.");
                hintType = Custom.HintText.Type.PasswordStrengthWeak;
                break;
            }
            case PasswordTextField.PasswordStrength.PasswordStrengthMedium: {
                hintTitle = qsTr("Average");
                hintText = qsTr("Your password is good enough to proceed, but it is recommended to strengthen it further.");
                hintType = Custom.HintText.Type.PasswordStrengthMedium;
                break;
            }
            case PasswordTextField.PasswordStrength.PasswordStrengthGood: {
                hintTitle = qsTr("Strong");
                hintText = qsTr("This password will withstand most typical brute-force attacks. Please ensure that you will remember it.");
                hintType = Custom.HintText.Type.PasswordStrengthGood;
                break;
            }
            case PasswordTextField.PasswordStrength.PasswordStrengthStrong: {
                hintTitle = qsTr("Excellent");
                hintText = qsTr("This password will withstand most sophisticated brute-force attacks. Please ensure that you will remember it.");
                hintType = Custom.HintText.Type.PasswordStrengthStrong;
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
