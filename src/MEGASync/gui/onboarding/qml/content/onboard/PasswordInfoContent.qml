// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import PasswordStrengthChecker 1.0

PasswordInfoContentForm {
    id: passwordInfoContentForm

    property bool allChecked: upperLowerCaseChecked && numberSpecialCharacterChecked
    property bool upperLowerCaseChecked: (RegexExpressions.upperCaseLeters).test(password)
                                            && (RegexExpressions.lowerCaseLeters).test(password)
    property bool numberSpecialCharacterChecked: (RegexExpressions.numbers).test(password)
                                                 || (RegexExpressions.specialCharacters).test(password)
    property bool validPassword: checker.strength >= PasswordStrengthChecker.PASSWORD_STRENGTH_WEAK
                                    && checker.strength <= PasswordStrengthChecker.PASSWORD_STRENGTH_STRONG

    conditionUpperLowerCase.checked: upperLowerCaseChecked
    conditionNumberSpecialCharacter.checked: numberSpecialCharacterChecked

    PasswordStrengthChecker {
        id: checker

        password: passwordInfoContentForm.password
    }

}
