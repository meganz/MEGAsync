import QtQuick 2.0
import Components.TextFields 1.0 as MegaTextFields

Item {
    id: root
    property string test: "test";
    onTestChanged: {
        console.log(test);
    }

    enum PasswordStrength {
        PasswordStrengthVeryWeak,
        PasswordStrengthWeak,
        PasswordStrengthMedium,
        PasswordStrengthGood,
        PasswordStrengthStrong
    }

    function getPasswordStrength(password) {
        console.info("getPasswordStrength(password)" + password);
        var strength = password.length - 1;
        return strength > PasswordStrengthChecker.PasswordStrengthStrong
               ? PasswordStrengthChecker.PasswordStrengthStrong
               : strength;
    }
}
