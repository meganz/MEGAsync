// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import PasswordStrengthChecker 1.0

PasswordInfoContentForm {

    function allChecked() {
        return upperLowerCaseChecked && numberSpecialCharacterChecked;
    }

    function checkPasswordConditions(password) {
        strengthTitle.font.strikeout = (password.length > 8);
        upperLowerCaseChecked = hasUpperAndLowerCase(password);
        numberSpecialCharacterChecked = hasNumberOrSpecialCharacter(password);
    }

    function hasUpperAndLowerCase(password) {
        return ((RegexExpressions.upperCaseLeters).test(password))
                && ((RegexExpressions.lowerCaseLeters).test(password))
    }

    function hasNumberOrSpecialCharacter(password) {
        return ((RegexExpressions.numbers).test(password))
                || ((RegexExpressions.specialCharacters).test(password));
    }

    PasswordStrengthChecker {
        id: checker
    }

}
