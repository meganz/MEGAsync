import common 1.0

import PasswordStrengthChecker 1.0

PasswordInfoContentForm {
    id: root

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

        password: root.password
    }

}
