import QtQuick 2.0
import Components.TextFields 1.0 as MegaTextFields

Item {
    id: root

    enum PasswordStrength {
        PasswordStrengthVeryWeak,
        PasswordStrengthWeak,
        PasswordStrengthMedium,
        PasswordStrengthGood,
        PasswordStrengthStrong
    }

    property string test: "test"

    function getPasswordStrength(password) {
        console.info("getPasswordStrength(password)" + password);
        var strength = password.length - 1;
        return strength > PasswordStrengthChecker.PasswordStrengthStrong
               ? PasswordStrengthChecker.PasswordStrengthStrong
               : strength;
    }

    onTestChanged: {
        console.log(test);
    }
}
