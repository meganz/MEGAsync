import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.textFields 1.0
import components.texts 1.0

TextField {
    id: root

    property var next
    property var previous

    height: 72 + 2 * sizes.focusBorderWidth
    width: 60 + 2 * sizes.focusBorderWidth
    sizes.focusBorderWidth: 4

    textField {
        height: root.height
        padding: 0
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        validator: RegExpValidator { regExp: RegexExpressions.digit2FA }
        font {
            pixelSize: Text.Size.Huge
            weight: Font.Bold
        }
    }

    textField.onTextChanged: {
        var isCharacterEntered = textField.text.length !== 0;
        if(isCharacterEntered && next !== undefined) {
            next.textField.focus = true;
        } else {
            textField.focus = false;
        }
        textField.horizontalAlignment = isCharacterEntered ? TextInput.AlignHCenter : TextInput.AlignLeft;
    }

    onFocusChanged: {
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
