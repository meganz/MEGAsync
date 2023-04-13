// System
import QtQuick 2.12
import QtQuick.Controls 2.12

// Local
import Components 1.0 as Custom
import Common 1.0

Custom.TextField {

    property var next
    property var previous

    height: 72
    width: 60

    textField.validator: RegExpValidator { regExp: RegexExpressions.digit2FA }
    textField.height: height + 6 // add the focus border size (3 up + 3 down)
    textField.font.pixelSize: 48
    textField.font.weight: Font.DemiBold
    textField.leftPadding: 17
    textField.bottomPadding: 10

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

    onPastePressed: {
        pastePin();
    }

}
