// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import PasswordStrengthChecker 1.0

PasswordInfoContentForm {

    function allChecked() {
        return upperLowerCaseChecked && numberSpecialCharacterChecked && longerChecked;
    }

    function checkPasswordConditions(password) {
        if(password.length < 8) {
            strengthTitle.text = OnboardingStrings.passwordAtleast8Chars;
            strenghtIcon.visible = false;
        } else {
            strenghtIcon.visible = true;
            setPasswordStrengthTitle(password);
        }

        upperLowerCaseChecked = hasUpperAndLowerCase(password);
        numberSpecialCharacterChecked = hasNumberOrSpecialCharacter(password);
        longerChecked = hasLongSize(password);
    }

    function hasUpperAndLowerCase(password) {
        return ((RegexExpressions.upperCaseLeters).test(password))
                && ((RegexExpressions.lowerCaseLeters).test(password))
    }

    function hasNumberOrSpecialCharacter(password) {
        return ((RegexExpressions.numbers).test(password))
                || ((RegexExpressions.specialCharacters).test(password));
    }

    function hasLongSize(password) {
        return password.length > 16;
    }

    function setPasswordStrengthTitle(password) {
        switch(checker.getPasswordStrength(password)) {
            case PasswordStrengthChecker.PasswordStrengthVeryWeak:
                strengthTitle.text = OnboardingStrings.passwordStrengthVeryWeak;
                strenghtIcon.source = Images.passwordVeryWeak;
                break;
            case PasswordStrengthChecker.PasswordStrengthWeak:
                strengthTitle.text = OnboardingStrings.passwordStrengthWeak;
                strenghtIcon.source = Images.passwordWeak;
                break;
            case PasswordStrengthChecker.PasswordStrengthMedium:
                strengthTitle.text = OnboardingStrings.passwordStrengthMedium;
                strenghtIcon.source = Images.passwordAverage;
                break;
            case PasswordStrengthChecker.PasswordStrengthGood:
                strengthTitle.text = OnboardingStrings.passwordStrengthGood;
                strenghtIcon.source = Images.passwordGood;
                break;
            case PasswordStrengthChecker.PasswordStrengthStrong:
                strengthTitle.text = OnboardingStrings.passwordStrengthStrong;
                strenghtIcon.source = Images.passwordStrong;
                break;
            default:
                console.error("PasswordInfoContent::setPasswordStrengthTitle value not found -> "
                              + strength);
                break;
        }
    }

    PasswordStrengthChecker {
        id: checker
    }

}
