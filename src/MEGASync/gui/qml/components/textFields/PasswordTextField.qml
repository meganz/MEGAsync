// System
import QtQuick 2.12

// Local
import Components 1.0 as Custom
import Common 1.0
import Onboarding 1.0
import Onboard 1.0

Custom.TextField {

    property bool showHint: false

    enum PasswordStrength{
        PASSWORD_STRENGTH_VERYWEAK = 0,
        PASSWORD_STRENGTH_WEAK = 1,
        PASSWORD_STRENGTH_MEDIUM = 2,
        PASSWORD_STRENGTH_GOOD = 3,
        PASSWORD_STRENGTH_STRONG = 4
    }

    textField.echoMode: TextInput.Password
    textField.onTextChanged: {
        rightIcon.visible = text.length;
        hint.visible = text.length && showHint;

        if(!showHint || !text.length)
            return;

        var strength = Onboarding.getPasswordStrength(text)
        switch(strength)
        {
        case PasswordTextField.PasswordStrength.PASSWORD_STRENGTH_VERYWEAK:
        {
            hint.title = OnboardingStrings.tooWeakPasswordTitle;
            hint.text = OnboardingStrings.tooWeakPasswordText;
            hint.titleColor = Styles.textError
            hint.iconColor = Styles.indicatorPink
            hint.iconSource = Images.passwordVeryWeak
            break;
        }
        case PasswordTextField.PasswordStrength.PASSWORD_STRENGTH_WEAK:
        {
            hint.title = OnboardingStrings.weakPasswordTitle;
            hint.text = OnboardingStrings.weakPasswordText;
            hint.titleColor = Styles.textError
            hint.iconColor = Styles.supportError
            hint.iconSource = Images.passwordWeak
            break;
        }
        case PasswordTextField.PasswordStrength.PASSWORD_STRENGTH_MEDIUM:
        {
            hint.title = OnboardingStrings.averagePasswordTitle;
            hint.text = OnboardingStrings.averagePasswordText;
            hint.titleColor = Styles.supportWarning
            hint.iconColor = Styles.supportWarning
            hint.iconSource = Images.passwordAverage
            break;
        }
        case PasswordTextField.PasswordStrength.PASSWORD_STRENGTH_GOOD:
        {
            hint.title = OnboardingStrings.strongPasswordTitle;
            hint.text = OnboardingStrings.strongPasswordText;
            hint.titleColor = Styles.textSuccess
            hint.iconColor = Styles.supportSuccess
            hint.iconSource = Images.passwordGood
            break;
        }
        case PasswordTextField.PasswordStrength.PASSWORD_STRENGTH_STRONG:
        {
            hint.title = OnboardingStrings.excelentPasswordTitle;
            hint.text = OnboardingStrings.excelentPasswordText;
            hint.titleColor = Styles.supportSuccess
            hint.iconColor = Styles.supportSuccess
            hint.iconSource = Images.passwordStrong
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
