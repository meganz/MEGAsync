// System
import QtQuick 2.12

// Local
import Components.TextFields 1.0 as MegaTextFields
import Common 1.0

MegaTextFields.TextField {

    function valid() {
        var validEmailRegex = RegexExpressions.email;
        return text.match(validEmailRegex);
    }

    textField.validator: RegExpValidator {
        regExp: RegexExpressions.email
    }

    hint.icon: Images.alertTriangle

    textField.onTextChanged: {
        error = false;
        hint.visible = false;
    }
}
