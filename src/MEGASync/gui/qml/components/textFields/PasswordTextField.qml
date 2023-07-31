// System
import QtQuick 2.12

// Local
import Components.TextFields 1.0 as MegaTextFields
import Common 1.0
import PasswordStrengthChecker 1.0

MegaTextFields.TextField {

    property bool showHint: false

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
        if(!showHint)
        {
            return;
        }
        rightIcon.visible = (text.length > 0);
        error = false;
        hint.visible = text.length > 0 && showHint;
        if(!(text.length > 0) || !showHint) {
            return;
        }

        var strength = strengthCheckerLoader.item.getPasswordStrength(text);
        switch(strength) {
        case PasswordStrengthChecker.PasswordStrengthVeryWeak: {
            hint.title = qsTr("Too weak");
            hint.text = qsTr("Your password needs to be at least 8 characters long.");
            hint.styles = veryWeak;
            hint.icon = Images.passwordVeryWeak
            break;
        }
        case PasswordStrengthChecker.PasswordStrengthWeak: {
            hint.title = qsTr("Weak");
            hint.text = qsTr("Your password is easily guessed. Try making your password longer. Combine uppercase and lowercase letters. Add special characters. Do not use names or dictionary words.");
            hint.styles = weak;
            hint.icon = Images.passwordWeak;
            break;
        }
        case PasswordStrengthChecker.PasswordStrengthMedium: {
            hint.title = qsTr("Average");
            hint.text = qsTr("Your password is good enough to proceed, but it is recommended to strengthen it further.");
            hint.styles = medium;
            hint.icon = Images.passwordAverage;
            break;
        }
        case PasswordStrengthChecker.PasswordStrengthGood: {
            hint.title = qsTr("Strong");
            hint.text = qsTr("This password will withstand most typical brute-force attacks. Please ensure that you will remember it.");
            hint.styles = good;
            hint.icon = Images.passwordGood;
            break;
        }
        case PasswordStrengthChecker.PasswordStrengthStrong: {
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
        if(textField.echoMode === TextInput.Password) {
            textField.echoMode = TextInput.Normal;
            rightIcon.source = "images/eye-off.svg";
        } else if(textField.echoMode === TextInput.Normal) {
            textField.echoMode = TextInput.Password;
            rightIcon.source = "images/eye.svg";
        }
    }

    onShowHintChanged: {
        if(showHint)
        {
            strengthCheckerLoader.sourceComponent = strengthCheckerComponent
        }
    }

    Loader {
        id: strengthCheckerLoader
    }

    Component{
        id:strengthCheckerComponent
        PasswordStrengthChecker{
        }
    }
}
