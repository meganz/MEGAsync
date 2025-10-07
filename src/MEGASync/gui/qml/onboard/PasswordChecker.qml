import QtQuick 2.15

import common 1.0

import PasswordStrengthChecker 1.0

Item {
    id: root

    property alias password: checker.password

    property bool allChecked: upperLowerCaseChecked && numberSpecialCharacterChecked
    property bool upperLowerCaseChecked: (RegexExpressions.upperCaseLeters).test(checker.password)
                                            && (RegexExpressions.lowerCaseLeters).test(checker.password)
    property bool numberSpecialCharacterChecked: (RegexExpressions.numbers).test(checker.password)
                                                 || (RegexExpressions.specialCharacters).test(checker.password)
    property bool validPassword: checker.strength >= PasswordStrengthChecker.PASSWORD_STRENGTH_WEAK
                                    && checker.strength <= PasswordStrengthChecker.PASSWORD_STRENGTH_STRONG

    PasswordStrengthChecker {
        id: checker
    }

}
