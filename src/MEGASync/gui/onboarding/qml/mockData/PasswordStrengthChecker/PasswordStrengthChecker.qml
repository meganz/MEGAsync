import QtQuick 2.0

Item {

    enum PasswordStrength {
        PASSWORD_STRENGTH_VERYWEAK = 0,
        PASSWORD_STRENGTH_WEAK = 1,
        PASSWORD_STRENGTH_MEDIUM = 2,
        PASSWORD_STRENGTH_GOOD = 3,
        PASSWORD_STRENGTH_STRONG = 4
    }

    property string password: ""
    property int strength: PasswordStrengthChecker.PASSWORD_STRENGTH_VERYWEAK

}
