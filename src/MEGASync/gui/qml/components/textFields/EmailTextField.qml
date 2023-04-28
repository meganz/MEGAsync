// System
import QtQuick 2.12

// Local
import Components 1.0 as Custom
import Common 1.0

Custom.TextField {

    function valid() {
        var validEmailRegex = RegexExpressions.email;
        return text.match(validEmailRegex);
    }

    textField.validator: RegExpValidator {
        regExp: RegexExpressions.email
    }

    type: Custom.TextField.Type.Error
    hintType: Custom.HintText.Type.Error
}
