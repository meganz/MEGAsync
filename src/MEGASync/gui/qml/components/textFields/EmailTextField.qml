import common 1.0

import components.textFields 1.0

TextField {
    id: root

    function valid() {
        var validEmailRegex = RegexExpressions.email;
        return text.match(validEmailRegex);
    }

    hint.icon: Images.alertTriangle
}
