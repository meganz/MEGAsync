// System
import QtQuick 2.15
import QtQuick.Controls 2.15

// Local
import Common 1.0
import Components.TextFields 1.0 as MegaTextFields
import Components.Texts 1.0 as MegaTexts

MegaTextFields.TextField {
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
            pixelSize: MegaTexts.Text.Huge
            weight: Font.DemiBold
            family: "Poppins"
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
