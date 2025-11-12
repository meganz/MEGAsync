pragma Singleton
import QtQuick 2.15

import common 1.0

QtObject {

    readonly property string forbiddenSpecialCharacters : "\" * / : &lt; &gt; ? \\ |"

    enum Error {
        SPECIAL_CHARACTERS = 0,
        TOO_LONG = 1,
        NAME_EXIST = 2,
        NAME_EMPTY = 3,
        NONE = 4
    }

    function getErrorCode(existingDeviceChecker, name) {
        if (name.length === 0) {
            return DeviceName.Error.NAME_EMPTY;
        }
        else if (name.length > 32) {
            return DeviceName.Error.TOO_LONG;
        }
        else if (name.match(RegexExpressions.deviceNameSpecialCharacters)) {
            return DeviceName.Error.SPECIAL_CHARACTERS;
        } 
        else if (existingDeviceChecker !== null && existingDeviceChecker.deviceNameAlreadyExists(name)) {
            return DeviceName.Error.NAME_EXIST;
        }
        return DeviceName.Error.NONE;
    }

    function getErrorString(error) {
        switch(error) {
        case DeviceName.Error.SPECIAL_CHARACTERS:
                return Strings.deviceNameSpecialCharactersErr.arg(forbiddenSpecialCharacters);
            case DeviceName.Error.TOO_LONG:
                return Strings.deviceNameTooLongErr;
            case DeviceName.Error.NAME_EMPTY:
                return Strings.deviceNameEmptyErr;
            case DeviceName.Error.NAME_EXIST:
                return Strings.deviceNameExistErr;
            default:
                return "";
        }
    }

    function showErrorRoutine(textField, error) {
        textField.error = error !== DeviceName.Error.NONE;
        textField.hint.visible = error !== DeviceName.Error.NONE;
        textField.hint.text = getErrorString(error);
    }
}
