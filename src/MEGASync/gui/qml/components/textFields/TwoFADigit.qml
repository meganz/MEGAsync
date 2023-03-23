import QtQuick 2.12
import QtQuick.Controls 2.12

import Components 1.0 as Custom

Custom.TextField {

    property var next
    property var previous

    textField.validator: RegExpValidator { regExp: /^[0-9]{1}$/ }
    fieldHeight: 72
    width: 60
    font.pixelSize: height - (fieldHeight / 2)

    textField.onTextChanged: {
        if(textField.text.length !== 0 && next !== undefined) {
            next.textField.focus = true;
        } else {
            textField.focus = false;
        }
    }

    textField.onFocusChanged: {
        if(textField.focus) {
            textField.select(0, 1);
        }
    }

    onBackPressed: {
        if(previous !== undefined) {
            previous.textField.focus = true;
        }
    }
}
