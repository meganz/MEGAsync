import common 1.0

import PasswordStrengthChecker 1.0

PasswordInfoContentForm {
    id: root

    property alias allChecked: passChecker.allChecked
    property alias validPassword: passChecker.validPassword

    conditionUpperLowerCase.checked: passChecker.upperLowerCaseChecked
    conditionNumberSpecialCharacter.checked: passChecker.numberSpecialCharacterChecked

    PasswordChecker {
        id: passChecker

        password: root.password
    }
}
