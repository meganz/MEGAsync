// System
import QtQuick 2.15

// Local
import Components.TextFields 1.0 as MegaTextFields
import Common 1.0

MegaTextFields.TextField {

    function valid() {
        var validEmailRegex = RegexExpressions.email;
        return text.match(validEmailRegex);
    }

    hint.icon: Images.alertTriangle
}
